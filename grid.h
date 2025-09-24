#pragma once
#include <raylib.h>
#include <iostream>
#include <vector>
#include "colors.h"
#include "blocks.h"
using namespace std;

class Grid {
public:
    Grid();
    void Initialize();
    void Print();
    int grid [20][10];
    void Draw();
    bool IsCellOutside(int row, int column);
    bool IsCellEmpty(int row, int column);
    bool IsRowFull(int row);
    void ClearRow(int row);
    void MoveRowDown(int row, int numsRows);
    int ClearFullRows();

private:
    int numRows;
    int numColumns;
    int cellSize;
    vector <Color> colors;

};