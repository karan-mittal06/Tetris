#include "blocks.h"
#include "position.h"
#include "iostream"
using namespace std;

Block::Block(){
    cellSize = 30;
    rotationState = 0;
    colors = GetCellColors();
    rowOffset = 0;
    columnOffset = 0;
}

void Block::Draw(){
    vector <Position> tiles = GetCellPosition();
    for (Position item: tiles){
        DrawRectangle(item.column * cellSize + 1, item.row * cellSize + 1, cellSize - 1, cellSize - 1, colors[id]);
    }
}

void Block::Move(int row, int column){
    rowOffset += row;
    columnOffset += column;
}

vector<Position> Block::GetCellPosition() {
    vector<Position> tiles = cells[rotationState];
    vector<Position> movedTiles;
    for (auto item : tiles) {
        Position newPos(item.row + rowOffset, item.column + columnOffset);
        movedTiles.push_back(newPos);
    }
    return movedTiles;
}


void Block::Rotate(){
    rotationState ++;

    if(rotationState == (int) cells.size()){
        rotationState = 0;
    }
}

void Block::UndoRotate(){
    rotationState--;

    if(rotationState == -1){
        rotationState = cells.size() - 1;
    }
}