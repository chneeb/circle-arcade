
#include "Ball.h"

Ball::Ball(int radius)
{
    this->radius = radius;
    x = 0;
    y = 0;
    speed_x = 0;
    speed_y = 0;
}

void Ball::Draw(C2DGraphics *graphics)
{
    graphics->DrawCircle(x,y, radius, RawToColor(0xff));
}

void Ball::Move() {
    x += speed_x;
    y += speed_y;

    if (y + radius  >= court->getOriginY() + court->getHeight() || y - radius <= court->getOriginY()) {
        FlipVerticalDirection();
    }
        
    // Cpu / left side wins
    if (x + radius >= court->getWidth()) {
        court->IncreaseLeftScore();
        ResetBall();
    }

    if (x - radius <= 0) {
        court->getRightScore();
        ResetBall();
    }
}

void Ball::ResetBall() {
    x = court->getWidth() / 2;
    y = court->getHeight() / 2;

    int speed_choices[2] = {-1, 1};
    speed_x *= speed_choices[rnd.GetNumber() % 2];
    speed_y *= speed_choices[rnd.GetNumber() % 2];
}

void Ball::FlipHorisontalDirection()
{
    speed_x *= -1;
}

void Ball::FlipVerticalDirection()
{
    speed_y *= -1;
}

void Ball::setPosition(int x, int y)
{
    this->x = x;
    this->y = y;
}

void Ball::setSpeed(int x_speed, int y_speed)
{
    this->speed_x = x_speed;
    this->speed_y = y_speed;
}

void Ball::setCourt(Court *court) {
    this->court = court;
}
