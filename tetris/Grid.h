#pragma once

#include <circle/2dgraphics.h>

#define COLUMNS 10
#define ROWS 20
#define SPACING 11

class Grid
{
public:
    Grid(int cellSize = 30);    
    void Initialize();
    void Draw(C2DGraphics *graphics);
    bool IsCellOutside(int row, int column);
    bool IsCellEmpty(int row, int column);
    int ClearFullRows();
    int grid[ROWS][COLUMNS];   
    int GetWidth();
    int GetHeight();

private:
    bool IsRowFull(int row);
    void ClearRow(int row);
    void MoveRowDown(int row, int numRows);
    int numRows;
    int numCols;
    int cellSize;
    T2DColor* colors;
};