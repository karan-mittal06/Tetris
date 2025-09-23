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
    bool IsBlockOutside();
    Grid grid;

private:
    vector <Block> blocks;
    Block currentblock;
    Block nextblock;
};
