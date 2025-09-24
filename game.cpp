#include "game.h"
#include <random>

Game::Game(){
    grid = Grid();
    blocks = GetAllBlocks();
    currentblock = GetRandomBlock();
    nextblock = GetRandomBlock();
    gameOver = false;
}

Block Game::GetRandomBlock(){
    if(blocks.empty()) blocks = GetAllBlocks();
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, blocks.size() - 1);

    int randomIndex = dist(gen);
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

    if(gameOver && keypressed != 0){
        gameOver = false;
        Reset();
    }

    switch(keypressed){
        case KEY_LEFT:
        MoveLeft();
        break;

        case KEY_RIGHT:
        MoveRight();
        break;

        case KEY_DOWN:
        MoveDown();
        break;

        case KEY_UP:
        RotateBlock();
        break;

        default:
        break;
    }
}

void Game::MoveLeft(){
    if(!gameOver){
        currentblock.Move(0,-1);
        if(IsBlockOutside() || BlockFits() == false) currentblock.Move(0,1);
    }
}

void Game::MoveRight(){
    if(!gameOver){
        currentblock.Move(0,1);
        if(IsBlockOutside() || BlockFits() == false) currentblock.Move(0,-1);
    }
}

void Game::MoveDown(){
    if(!gameOver){
        currentblock.Move(1,0);
        if(IsBlockOutside() || BlockFits() == false) {
            currentblock.Move(-1,0);
            LockBlock();
        }
    }
}

void Game::LockBlock(){
    for(const auto item: currentblock.GetCellPosition()){
        grid.grid[item.row][item.column] = currentblock.id;
    }
    currentblock = nextblock;
    if(BlockFits() == false){
        gameOver = true;
    }
    nextblock = GetRandomBlock();
    grid.ClearFullRows();
}

bool Game::IsBlockOutside()
{
    for(const auto item: currentblock.GetCellPosition()){
        if(grid.IsCellOutside(item.row, item.column)){
            return true;
        }
    }
    return false;
}

void Game::RotateBlock(){
    currentblock.Rotate();

    if(IsBlockOutside()){
        currentblock.UndoRotate();
    }
}

bool Game::BlockFits(){
    for(const auto item: currentblock.GetCellPosition()){
        if(grid.IsCellEmpty(item.row,item.column) == false){
            return false;
        }
    }
    return true;
}

void Game::Reset(){
    grid.Initialize();
    blocks = GetAllBlocks();
    currentblock = GetRandomBlock();
    nextblock = GetRandomBlock();
}