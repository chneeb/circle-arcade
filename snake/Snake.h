
#pragma once
#include "../utils/Color.h"
#include "../utils/Math.h"
#include "../utils/Deque.h"
#include <circle/2dgraphics.h>


class Snake 
{

private:
    
    int cellSize;    
    int offset; 

public:

    Vector2D direction = {1, 0};
    bool addSegment = false;   
    
    Deque body;
    Snake(int size = 0);
    void Draw(C2DGraphics *graphics);
    void Update();
    void Reset();
};

