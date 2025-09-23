#include "game.h"
#include <random>

Game::Game(){
    grid = Grid();
    blocks = GetAllBlocks();
    currentblock = GetRandomBlock();
    nextblock = GetRandomBlock();
}

Block Game::GetRandomBlock(){
    if(blocks.empty()) blocks = GetAllBlocks();
    int randomIndex = rand() % blocks.size();
    Block block = blocks[randomIndex];
    blocks.erase(blocks.begin() + randomIndex);
    return block;
}

vector <Block> Game::GetAllBlocks(){
    return {Iblock(), Oblock(), Tblock(), Jblock(), Lblock(), Sblock(), Zblock()};
}

void Game::Draw(){
    grid.Draw();
    currentblock.Draw();
}

void Game::HandleInput(){
    int keypressed = GetKeyPressed();

    switch(keypressed){
        case KEY_LEFT:
        if(!IsBlockOutside()) MoveLeft();
        break;

        case KEY_RIGHT:
        if(!IsBlockOutside()) MoveRight();
        break;

        case KEY_DOWN:
        if(!IsBlockOutside()) MoveDown();
        break;

        default:
        break;
    }
}

void Game::MoveLeft(){
    currentblock.Move(0,-1);
}

void Game::MoveRight(){
    currentblock.Move(0,1);
}

void Game::MoveDown(){
    currentblock.Move(1,0);
}

bool Game::IsBlockOutside(){
    vector <Position> currentPos = currentblock.GetCellPosition();
    return grid.IsCellOutside(currentPos);
}