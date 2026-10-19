#pragma once
#include "grid.h"
#include "blocks.cpp"
using namespace std;

class Game{
public:
    Game();
    Block GetRandomBlock();
    vector<Block> GetAllBlocks();
    void Draw();
    void HandleInput();
    void MoveLeft();
    void MoveRight();
    void MoveUp();
    void MoveDown();
    Grid grid;
    bool gameOver;
    int score;
    int level;
    int totalLines;

private:
    vector <Block> blocks;
    Block currentblock;
    Block nextblock;
    void LockBlock();
    void UpdateScore(int clearedLines);
    void UpdateLevel();
    void DrawStats();
    void DrawGameOverScreen();
    bool IsBlockOutside();
    void RotateBlock();
    bool BlockFits();
    void Reset();
};
