#pragma once
#include "../utils/Color.h"
#include "../utils/Math.h"
#include <circle/2dgraphics.h>

class ObstacleBlock {
    public:
        ObstacleBlock() {};
        ObstacleBlock(Vector2D position);
        void Draw(C2DGraphics *graphics);
        Rectangle getRect();
    private:
        Vector2D position;
};