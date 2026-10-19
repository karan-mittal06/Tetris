#include <raylib.h>
#include "game.h"
#include "network_manager.h"

#include <atomic>
#include <string>
#include <thread>

double lastUpdateTime = 0;

bool EventTriggered(double interval){
    double currentTime = GetTime();

    if(currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }

    return false;
}

namespace {
enum class Screen { MainMenu, SinglePlayer, MultiplayerMenu, Join, HostWaiting, Match };
constexpr std::uint16_t networkPort = 45678;

void DrawCentered(const char* text, int y, int size, Color color, int width = 900) {
    DrawText(text, width / 2 - MeasureText(text, size) / 2, y, size, color);
}

void DrawBoardSnapshot(const PlayerSnapshot& snapshot, int x, int y, int cellSize) {
    DrawRectangle(x - 2, y - 2, 10 * cellSize + 4, 20 * cellSize + 4, DARKGRAY);
    for(int row = 0; row < 20; row++) {
        for(int column = 0; column < 10; column++) {
            int value = snapshot.board[row * 10 + column];
            Color color = value == 0 ? Color{25, 25, 60, 255} : GetCellColors()[value];
            DrawRectangle(x + column * cellSize, y + row * cellSize,
                          cellSize - 1, cellSize - 1, color);
        }
    }
}

PlayerSnapshot MakeSnapshot(const Game& game) {
    PlayerSnapshot snapshot;
    for(int row = 0; row < 20; row++) {
        for(int column = 0; column < 10; column++) {
            snapshot.board[row * 10 + column] =
                static_cast<std::uint8_t>(game.grid.grid[row][column]);
        }
    }
    snapshot.score = game.score;
    snapshot.level = game.level;
    snapshot.lines = game.totalLines;
    snapshot.gameOver = game.gameOver;
    return snapshot;
}

void DrawMatchResult(const Game& game, const PlayerSnapshot& remote, bool disconnected) {
    const char* result = disconnected ? "Opponent Disconnected"
        : (game.gameOver ? "You Lose" : (remote.gameOver ? "You Win" : "Match Over"));
    DrawRectangle(0, 0, 900, 600, Color{0, 0, 0, 180});
    DrawRectangle(180, 190, 540, 180, Color{25, 25, 65, 255});
    DrawCentered(result, 225, 38, SKYBLUE);
    DrawCentered("Press Enter to return to the menu", 300, 22, WHITE);
}
}

int main() {
    Color background = {44, 44, 127, 255};

    InitWindow(900, 600, "raylib Tetris");
    SetTargetFPS(60);

    Screen screen = Screen::MainMenu;
    int menuSelection = 0;
    std::string hostIp;
    std::string connectionError;
    NetworkManager network;
    Game game;
    PlayerSnapshot remote;
    bool matchEnded = false;
    std::atomic<bool> hostFinished = false;
    std::thread hostThread;

    auto startHost = [&]() {
        hostFinished = false;
        connectionError.clear();
        hostThread = std::thread([&network, &hostFinished]() {
            std::string error;
            network.Host(networkPort, error);
            hostFinished = true;
        });
    };

    while(WindowShouldClose() == false){
        if(screen == Screen::MainMenu) {
            int key = GetKeyPressed();
            if(key == KEY_UP || key == KEY_DOWN) menuSelection = 1 - menuSelection;
            if(key == KEY_ENTER) {
                screen = menuSelection == 0 ? Screen::SinglePlayer : Screen::MultiplayerMenu;
                game = Game();
            }
            if(key == KEY_ESCAPE) break;
        } else if(screen == Screen::SinglePlayer) {
            game.HandleInput();
            if(EventTriggered(0.2)) game.MoveDown();
            if(IsKeyPressed(KEY_ESCAPE)) screen = Screen::MainMenu;
        } else if(screen == Screen::MultiplayerMenu) {
            int key = GetKeyPressed();
            if(key == KEY_UP || key == KEY_DOWN) menuSelection = 1 - menuSelection;
            if(key == KEY_ENTER) {
                if(menuSelection == 0) {
                    screen = Screen::HostWaiting;
                    startHost();
                } else {
                    hostIp.clear();
                    connectionError.clear();
                    screen = Screen::Join;
                }
            }
            if(key == KEY_ESCAPE) screen = Screen::MainMenu;
        } else if(screen == Screen::Join) {
            int character = GetCharPressed();
            while(character > 0) {
                if((character >= '0' && character <= '9') || character == '.') {
                    hostIp += static_cast<char>(character);
                }
                character = GetCharPressed();
            }
            if(IsKeyPressed(KEY_BACKSPACE) && !hostIp.empty()) hostIp.pop_back();
            if(IsKeyPressed(KEY_ENTER)) {
                if(network.Join(hostIp, networkPort, connectionError)) {
                    game = Game();
                    matchEnded = false;
                    screen = Screen::Match;
                }
            }
            if(IsKeyPressed(KEY_ESCAPE)) screen = Screen::MultiplayerMenu;
        } else if(screen == Screen::HostWaiting) {
            if(hostFinished) {
                if(hostThread.joinable()) hostThread.join();
                if(network.IsConnected()) {
                    game = Game();
                    matchEnded = false;
                    screen = Screen::Match;
                } else {
                    screen = Screen::MultiplayerMenu;
                }
            }
            if(IsKeyPressed(KEY_ESCAPE)) {
                network.Disconnect();
                if(hostThread.joinable()) hostThread.join();
                screen = Screen::MultiplayerMenu;
            }
        } else if(screen == Screen::Match) {
            int key = GetKeyPressed();
            if(!matchEnded) {
                if(key == KEY_LEFT) game.MoveLeft();
                if(key == KEY_RIGHT) game.MoveRight();
                if(key == KEY_UP) game.MoveUp();
                if(key == KEY_DOWN) game.MoveDown();
                if(EventTriggered(0.2)) game.MoveDown();

                network.SendSnapshot(MakeSnapshot(game));
                network.GetLatestSnapshot(remote);
                matchEnded = game.gameOver || remote.gameOver || network.IsDisconnected();
            } else if(key == KEY_ENTER || key == KEY_ESCAPE) {
                network.Disconnect();
                screen = Screen::MainMenu;
                matchEnded = false;
            }
        }

        BeginDrawing();
        ClearBackground(background);
        if(screen == Screen::MainMenu) {
            DrawCentered("TETRIS", 100, 48, SKYBLUE);
            DrawCentered(menuSelection == 0 ? "> Single Player" : "  Single Player", 230, 28, WHITE);
            DrawCentered(menuSelection == 1 ? "> Multiplayer" : "  Multiplayer", 290, 28, WHITE);
            DrawCentered("Press Enter to select", 450, 20, LIGHTGRAY);
        } else if(screen == Screen::MultiplayerMenu) {
            DrawCentered("MULTIPLAYER", 100, 42, SKYBLUE);
            DrawCentered(menuSelection == 0 ? "> Host Game" : "  Host Game", 230, 28, WHITE);
            DrawCentered(menuSelection == 1 ? "> Join Game" : "  Join Game", 290, 28, WHITE);
            DrawCentered("Escape to go back", 450, 20, LIGHTGRAY);
        } else if(screen == Screen::SinglePlayer) {
            game.Draw();
        } else if(screen == Screen::Join) {
            DrawCentered("JOIN GAME", 100, 42, SKYBLUE);
            DrawCentered("Enter host IP address:", 220, 24, WHITE);
            DrawCentered(hostIp.c_str(), 270, 30, YELLOW);
            DrawCentered("Enter to connect | Escape to cancel", 450, 20, LIGHTGRAY);
            if(!connectionError.empty()) DrawCentered(connectionError.c_str(), 380, 20, RED);
        } else if(screen == Screen::HostWaiting) {
            DrawCentered("HOST GAME", 90, 42, SKYBLUE);
            DrawCentered(("Your LAN IP: " + network.GetLocalIp()).c_str(), 210, 24, WHITE);
            DrawCentered("Port: 45678", 250, 24, WHITE);
            DrawCentered("Waiting for one player...", 340, 24, YELLOW);
            DrawCentered("Escape to cancel", 450, 20, LIGHTGRAY);
        } else if(screen == Screen::Match) {
            game.Draw();
            DrawBoardSnapshot(remote, 600, 150, 15);
            DrawText("Opponent", 600, 110, 24, WHITE);
            DrawText(TextFormat("Score: %d", remote.score), 760, 170, 20, WHITE);
            DrawText(TextFormat("Level: %d", remote.level), 760, 205, 20, WHITE);
            DrawText(TextFormat("Lines: %d", remote.lines), 760, 240, 20, WHITE);
            if(matchEnded) DrawMatchResult(game, remote, network.IsDisconnected());
        }
        EndDrawing();
    }

    network.Disconnect();
    if(hostThread.joinable()) hostThread.join();
    CloseWindow();
}