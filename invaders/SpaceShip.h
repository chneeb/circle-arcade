
#pragma once

#include "../utils/Math.h"
#include "../utils/Image.h"
#include "../utils/Array.h"
#include <circle/2dgraphics.h>
#include <circle/bcmrandom.h>
#include <circle/timer.h>

#include "Laser.h"

class Spaceship{
    public:
        Spaceship(Rectangle screen = {0,0,640,480});
        ~Spaceship();
        void Draw(C2DGraphics *graphics);
        void MoveLeft();
        void MoveRight();
        bool FireLaser(float ticks);
        Rectangle GetRect();
        void Reset();

        Array<Laser*> lasers;
        //Array<Laser> getLasers() {return lasers; };

    private:
        Image *image;
        Vector2D position;
        double lastFireTime;

        int screenWidth;
        int screenHeight;
        int speed;
        int laserSpeed;

        
        CString debugText; 
};