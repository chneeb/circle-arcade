
#pragma once
#include "../utils/Color.h"
#include "../utils/Math.h"
#include "../utils/Deque.h"
#include <circle/2dgraphics.h>
#include <circle/bcmrandom.h>

class Food
{

private:
    int cellCount;
    int cellSize;    
    int offset;
    
    CBcmRandomNumberGenerator rnd = CBcmRandomNumberGenerator();


public:
    Vector2D position;
    
    Food(int size = 0, int gridsize = 0);
    ~Food();
    void Draw(C2DGraphics *graphics);
    void SetRandomPos(Deque snakeBody);
private:
    Vector2D GenerateRandomCell();
    

    
};

