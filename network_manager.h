#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

struct PlayerSnapshot {
    std::array<std::uint8_t, 200> board{};
    std::int32_t score = 0;
    std::int32_t level = 1;
    std::int32_t lines = 0;
    bool gameOver = false;
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    bool Host(std::uint16_t port, std::string& error);
    bool Join(const std::string& hostIp, std::uint16_t port, std::string& error);
    bool IsConnected() const;
    bool IsDisconnected() const;
    std::string GetLocalIp() const;
    bool SendSnapshot(const PlayerSnapshot& snapshot);
    bool GetLatestSnapshot(PlayerSnapshot& snapshot);
    void Disconnect();

private:
    int listenFd;
    int socketFd;
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> stopping;
    mutable std::mutex sendMutex;
    mutable std::mutex snapshotMutex;
    PlayerSnapshot latestSnapshot;
    bool hasSnapshot;
    std::thread receiveThread;

    bool ConnectSocket(int fd, std::string& error);
    void ReceiveLoop();
    void CloseSocket();
};
