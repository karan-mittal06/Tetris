#include <raylib.h>
#include <iostream>
#include <vector>
#include "grid.h"
#include "colors.h"
using namespace std;

Grid::Grid() {
    numRows = 20;
    numColumns = 10;
    cellSize = 30;
    Initialize();
    colors = GetCellColors();
}

void Grid::Initialize(){
    for (int row = 0; row < numRows; row ++){
        for (int columns = 0; columns < numColumns; columns ++){
            grid[row][columns] = 0;
        }
    }
}

void Grid::Draw() {
    int cellValue = 0;
    for (int row = 0; row < numRows; row++){
        for (int column = 0; column < numColumns; column++){
            cellValue = grid[row][column];
            DrawRectangle(column * cellSize + 1, row * cellSize + 1, cellSize - 1, cellSize - 1, colors[cellValue]);
        }
    }
}

bool Grid::IsCellOutside(int row, int column){
    return (row < 0 || row >= numRows || column < 0 || column >= numColumns);
}

bool Grid::IsCellEmpty(int row, int column){
    if(grid[row][column] == 0){
        return true;
    }
    return false;
}

bool Grid::IsRowFull(int row){
    for(int col = 0; col < numColumns; col++){
        if(grid[row][col] == 0) return false;
    }
    return true;
}

void Grid::ClearRow(int row){
    for(int col = 0; col < numColumns; col++){
        grid[row][col] = 0;
    }
}

void Grid::MoveRowDown(int row, int upRows){
    for(int col = 0; col < numColumns; col++){
        grid[row+upRows][col] = grid[row][col];
        grid[row][col] = 0;
    }
}

int Grid::ClearFullRows(){
    int completed = 0;
    for(int row = numRows - 1; row >= 0; row--){
        if(IsRowFull(row)){
            ClearRow(row);
            completed++;
        }
        else if(completed > 0){
            MoveRowDown(row, completed);
        }
    }
    return completed;
}