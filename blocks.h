#pragma once
#include <vector>
#include <map>
#include "position.h"
#include "colors.h"
using namespace std;

class Block{
public:
    Block();
    int id;
    map <int, vector<Position>> cells; 
    void Draw();
    void Move(int row, int column);
    vector <Position> GetCellPosition();

private:
    int cellSize;
    int rotationState;
    vector <Color> colors;
    int rowOffset;
    int columnOffset;
};