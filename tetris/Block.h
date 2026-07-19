#pragma once
#include <circle/2dgraphics.h>
#include "Position.h"
#include "Colors.h"

class Block
{
public:
    Block(int cellSize = 30);
    void Draw(int offsetX, int offsetY, C2DGraphics *graphics);
    void Move(int rows, int columns);
    Position* GetCellPositions();
    void Rotate();
    void UndoRotation();
    int id;
    Position* cells[4];

private:
    int cellSize;
    int rotationState;
    TScreenColor* colors;
    int rowOffset;
    int columnOffset;
};