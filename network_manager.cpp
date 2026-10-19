#include "network_manager.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <algorithm>

namespace {
constexpr std::uint16_t kDefaultPort = 45678;
constexpr std::size_t kSnapshotSize = 200 + sizeof(std::int32_t) * 3 + 1;

bool SendAll(int fd, const void* data, std::size_t size) {
    const char* bytes = static_cast<const char*>(data);
    while(size > 0) {
        ssize_t sent = send(fd, bytes, size, 0);
        if(sent <= 0) return false;
        bytes += sent;
        size -= static_cast<std::size_t>(sent);
    }
    return true;
}

bool ReceiveAll(int fd, void* data, std::size_t size) {
    char* bytes = static_cast<char*>(data);
    while(size > 0) {
        ssize_t received = recv(fd, bytes, size, 0);
        if(received <= 0) return false;
        bytes += received;
        size -= static_cast<std::size_t>(received);
    }
    return true;
}

void WriteInt(std::uint8_t* output, std::int32_t value) {
    std::uint32_t networkValue = htonl(static_cast<std::uint32_t>(value));
    std::memcpy(output, &networkValue, sizeof(networkValue));
}

std::int32_t ReadInt(const std::uint8_t* input) {
    std::uint32_t networkValue = 0;
    std::memcpy(&networkValue, input, sizeof(networkValue));
    return static_cast<std::int32_t>(ntohl(networkValue));
}
}

NetworkManager::NetworkManager()
    : listenFd(-1), socketFd(-1), connected(false), disconnected(false), stopping(false),
      hasSnapshot(false) {}

NetworkManager::~NetworkManager() {
    Disconnect();
}

bool NetworkManager::Host(std::uint16_t port, std::string& error) {
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0) {
        error = "Could not create server socket.";
        return false;
    }

    int reuse = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port == 0 ? kDefaultPort : port);
    if(bind(listenFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 ||
       listen(listenFd, 1) < 0) {
        error = "Could not start the server.";
        close(listenFd);
        listenFd = -1;
        return false;
    }

    sockaddr_in clientAddress{};
    socklen_t clientLength = sizeof(clientAddress);
    int client = accept(listenFd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength);
    close(listenFd);
    listenFd = -1;
    if(client < 0) {
        error = "Waiting for a player failed.";
        return false;
    }

    return ConnectSocket(client, error);
}

bool NetworkManager::Join(const std::string& hostIp, std::uint16_t port, std::string& error) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        error = "Could not create client socket.";
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port == 0 ? kDefaultPort : port);
    if(inet_pton(AF_INET, hostIp.c_str(), &address.sin_addr) != 1) {
        error = "Invalid IPv4 address.";
        close(fd);
        return false;
    }

    if(connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        error = "Could not connect to host.";
        close(fd);
        return false;
    }
    return ConnectSocket(fd, error);
}

bool NetworkManager::ConnectSocket(int fd, std::string& error) {
    socketFd = fd;
    connected = true;
    disconnected = false;
    stopping = false;
    receiveThread = std::thread(&NetworkManager::ReceiveLoop, this);
    if(!connected) {
        error = "Connection failed.";
        return false;
    }
    return true;
}

bool NetworkManager::SendSnapshot(const PlayerSnapshot& snapshot) {
    if(!IsConnected()) return false;

    std::array<std::uint8_t, kSnapshotSize> packet{};
    std::copy(snapshot.board.begin(), snapshot.board.end(), packet.begin());
    WriteInt(packet.data() + 200, snapshot.score);
    WriteInt(packet.data() + 204, snapshot.level);
    WriteInt(packet.data() + 208, snapshot.lines);
    packet[212] = snapshot.gameOver ? 1 : 0;

    std::lock_guard<std::mutex> lock(sendMutex);
    if(!SendAll(socketFd, packet.data(), packet.size())) {
        disconnected = true;
        connected = false;
        return false;
    }
    return true;
}

void NetworkManager::ReceiveLoop() {
    std::array<std::uint8_t, kSnapshotSize> packet{};
    while(!stopping && ReceiveAll(socketFd, packet.data(), packet.size())) {
        PlayerSnapshot snapshot;
        std::copy(packet.begin(), packet.begin() + 200, snapshot.board.begin());
        snapshot.score = ReadInt(packet.data() + 200);
        snapshot.level = ReadInt(packet.data() + 204);
        snapshot.lines = ReadInt(packet.data() + 208);
        snapshot.gameOver = packet[212] != 0;

        {
            std::lock_guard<std::mutex> lock(snapshotMutex);
            latestSnapshot = snapshot;
            hasSnapshot = true;
        }
    }
    if(!stopping) {
        disconnected = true;
        connected = false;
    }
}

bool NetworkManager::GetLatestSnapshot(PlayerSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(snapshotMutex);
    if(!hasSnapshot) return false;
    snapshot = latestSnapshot;
    return true;
}

bool NetworkManager::IsConnected() const {
    return connected;
}

bool NetworkManager::IsDisconnected() const {
    return disconnected;
}

std::string NetworkManager::GetLocalIp() const {
    ifaddrs* interfaces = nullptr;
    if(getifaddrs(&interfaces) != 0) return "unknown";

    std::string address = "127.0.0.1";
    for(ifaddrs* current = interfaces; current != nullptr; current = current->ifa_next) {
        if(current->ifa_addr == nullptr || current->ifa_addr->sa_family != AF_INET) continue;
        auto* ipv4 = reinterpret_cast<sockaddr_in*>(current->ifa_addr);
        char buffer[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &ipv4->sin_addr, buffer, sizeof(buffer));
        if(std::strcmp(buffer, "127.0.0.1") != 0) {
            address = buffer;
            break;
        }
    }
    freeifaddrs(interfaces);
    return address;
}

void NetworkManager::CloseSocket() {
    if(socketFd >= 0) {
        shutdown(socketFd, SHUT_RDWR);
        close(socketFd);
        socketFd = -1;
    }
}

void NetworkManager::Disconnect() {
    stopping = true;
    connected = false;
    if(listenFd >= 0) {
        shutdown(listenFd, SHUT_RDWR);
        close(listenFd);
        listenFd = -1;
    }
    CloseSocket();
    if(receiveThread.joinable()) receiveThread.join();
}
