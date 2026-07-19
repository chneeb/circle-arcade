#include "Position.h"

Position::Position(int row, int column)
{
    this->row = row;
    this->column = column;
}

Position::Position()
{
    this->row = 0;
    this->column = 0;
}