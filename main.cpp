#include "raylib.h"
#include <iostream>
#include "game.h"
using namespace std;

int main() {
    Color background = {44, 44, 127, 255};
    
    InitWindow(300,600,"raylib Tetris");
    SetTargetFPS(60);

    Game game = Game();

    while(WindowShouldClose() == false){
        BeginDrawing();
        game.HandleInput();
        ClearBackground(background);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
}