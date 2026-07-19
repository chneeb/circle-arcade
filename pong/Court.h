#pragma once

#include "../utils/Color.h"
#include <circle/2dgraphics.h>
#include "../utils/Math.h"

class Court {
 public:   
    Court(int originX = 0, int originY = 0, int width = 0, int height = 0);
    ~Court();

    int getOriginX() {return x;};
    int getOriginY() {return y;};
    int getWidth() {return width;};
    int getHeight() {return height;};
    int getLeftScore() {return leftScore;};
    int getRightScore() {return rightScore;};

    void IncreaseLeftScore();
    void IncreaseRightScore();
    void ResetScore();
    void Draw(C2DGraphics *graphics);

private:
    int x;
    int y;
    int width;
    int height;
    Point center;
    int leftScore = 0;
    int rightScore = 0;
};