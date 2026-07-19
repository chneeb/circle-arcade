#pragma once

#include "../utils/Color.h"
#include <circle/2dgraphics.h>
#include "Ball.h"
#include "../utils/Math.h"
#include "../utils/Number.h"

class Paddle {
 protected:
    void LimitMovement();
    float x, y;
    float width, height;
    int speed;
    Court* court;

 public:
    Paddle();
    void Draw(C2DGraphics *graphics);
    void MoveUp();
    void MoveDown();
    void setCourt(Court *c);
    bool CheckCollisionWithBall(Ball ball);

    void setPosition(int x, int y);    
    void setSize(int width, int height);
    int getWidth() { return width; };
    int getHeight() { return height; };
    void setSpeed(int speed);
};