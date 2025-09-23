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
    bool IsCellOutside(vector <Position> tiles);

private:
    int numRows;
    int numColumns;
    int cellSize;
    vector <Color> colors;

};