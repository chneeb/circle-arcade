#include "Block.h"
#include "Position.h"

class LBlock : public Block
{
public:
    LBlock(int size): Block(size)
    {
        id = 1;
        cells[0] = new Position[4]{Position(0, 2), Position(1, 0), Position(1, 1), Position(1, 2)}; //  firstPosition;
        cells[1] = new Position[4]{Position(0, 1), Position(1, 1), Position(2, 1), Position(2, 2)};
        cells[2] = new Position[4]{Position(1, 0), Position(1, 1), Position(1, 2), Position(2, 0)};
        cells[3] = new Position[4]{Position(0, 0), Position(0, 1), Position(1, 1), Position(2, 1)};
        Move(0, 3);
    }
};

class JBlock : public Block
{
public:
    JBlock(int size): Block(size)
    {
        id = 2;
        cells[0] = new Position[4]{Position(0, 0), Position(1, 0), Position(1, 1), Position(1, 2)};
        cells[1] = new Position[4]{Position(0, 1), Position(0, 2), Position(1, 1), Position(2, 1)};
        cells[2] = new Position[4]{Position(1, 0), Position(1, 1), Position(1, 2), Position(2, 2)};
        cells[3] = new Position[4]{Position(0, 1), Position(1, 1), Position(2, 0), Position(2, 1)};
        Move(0, 3);
    }
};

class IBlock : public Block
{
public:
    IBlock(int size): Block(size)
    {
        id = 3;
        cells[0] = new Position[4]{Position(1, 0), Position(1, 1), Position(1, 2), Position(1, 3)};
        cells[1] = new Position[4]{Position(0, 2), Position(1, 2), Position(2, 2), Position(3, 2)};
        cells[2] = new Position[4]{Position(2, 0), Position(2, 1), Position(2, 2), Position(2, 3)};
        cells[3] = new Position[4]{Position(0, 1), Position(1, 1), Position(2, 1), Position(3, 1)};
        Move(-1, 3);
    }
};

class OBlock : public Block
{
public:
    OBlock(int size): Block(size)
    {
        id = 4;
        cells[0] = new Position[4]{Position(0, 0), Position(0, 1), Position(1, 0), Position(1, 1)};
        cells[1] = new Position[4]{Position(0, 0), Position(0, 1), Position(1, 0), Position(1, 1)};
        cells[2] = new Position[4]{Position(0, 0), Position(0, 1), Position(1, 0), Position(1, 1)};
        cells[3] = new Position[4]{Position(0, 0), Position(0, 1), Position(1, 0), Position(1, 1)};
        Move(0, 4);
    }
};

class SBlock : public Block
{
public:
    SBlock(int size): Block(size)
    {
        id = 5;
        cells[0] = new Position[4]{Position(0, 1), Position(0, 2), Position(1, 0), Position(1, 1)};
        cells[1] = new Position[4]{Position(0, 1), Position(1, 1), Position(1, 2), Position(2, 2)};
        cells[2] = new Position[4]{Position(1, 1), Position(1, 2), Position(2, 0), Position(2, 1)};
        cells[3] = new Position[4]{Position(0, 0), Position(1, 0), Position(1, 1), Position(2, 1)};
        Move(0, 3);
    }
};

class TBlock : public Block
{
public:
    TBlock(int size): Block(size)
    {
        id = 6;
        cells[0] = new Position[4]{Position(0, 1), Position(1, 0), Position(1, 1), Position(1, 2)};
        cells[1] = new Position[4]{Position(0, 1), Position(1, 1), Position(1, 2), Position(2, 1)};
        cells[2] = new Position[4]{Position(1, 0), Position(1, 1), Position(1, 2), Position(2, 1)};
        cells[3] = new Position[4]{Position(0, 1), Position(1, 0), Position(1, 1), Position(2, 1)};
        Move(0, 3);
    }
};

class ZBlock : public Block
{
public:
    ZBlock(int size): Block(size)
    {
        id = 7;
        cells[0] = new Position[4]{Position(0, 0), Position(0, 1), Position(1, 1), Position(1, 2)};
        cells[1] = new Position[4]{Position(0, 2), Position(1, 1), Position(1, 2), Position(2, 1)};
        cells[2] = new Position[4]{Position(1, 0), Position(1, 1), Position(2, 1), Position(2, 2)};
        cells[3] = new Position[4]{Position(0, 1), Position(1, 0), Position(1, 1), Position(2, 0)};
        Move(0, 3);
    }
};