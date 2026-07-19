#pragma once
#include "../utils/Color.h"
#include "../utils/Math.h"
#include <circle/2dgraphics.h>
#include <circle/string.h>

class Laser {
    public:
        
        Laser(Vector2D position = {0,0}, int speed = -1, int verticalHeight = 0);
        void Update();
        void Draw(C2DGraphics *graphics);
        Rectangle GetRect();
        bool active;
    private:
        Vector2D position;
        int speed;
        int screenHeight;
        CString debugText;
};