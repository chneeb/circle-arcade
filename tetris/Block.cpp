#include "Block.h"

Block::Block(int size)
{
    cellSize = size;
    rotationState = 0;
    colors = GetCellColors();
    rowOffset = 0;
    columnOffset = 0;
}

void Block::Draw(int offsetX, int offsetY, C2DGraphics *graphics)
{
    Position *tiles = GetCellPositions();
    for (int i = 0; i < 4; i++)
    {
        Position item = (Position)tiles[i];

        graphics->DrawRect(item.column * cellSize + offsetX, item.row * cellSize + offsetY, cellSize - 1, cellSize - 1, colors[id]);
    }
}

void Block::Move(int rows, int columns)
{
    rowOffset += rows;
    columnOffset += columns;
}

Position* Block::GetCellPositions()
{
    Position* tiles = (Position*)cells[rotationState];   
    Position *movedTiles = new Position[4];
    for (int i = 0; i < 4; i++)
    {
        Position item = (Position)tiles[i];
        Position newPos = Position(item.row + rowOffset, item.column + columnOffset);
        movedTiles[i] = newPos;
    }
    return movedTiles;
}

void Block::Rotate()
{
    rotationState++;
    if (rotationState == 4)
    {
        rotationState = 0;
    }
}

void Block::UndoRotation()
{
    rotationState--;
    if (rotationState == -1)
    {
        rotationState = 3;
    }
}