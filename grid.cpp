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

bool Grid::IsCellOutside(vector <Position> tiles){
    for(auto item: tiles){
        if(!(item.row >= 0 && item.row < numRows && item.column >= 0 && item.column < numColumns)){
            return true;
        }
    }
    return false;
}