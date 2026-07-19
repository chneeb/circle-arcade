#include "Colors.h"

const T2DColor darkGrey = COLOR16(15, 15, 15);
const T2DColor green = COLOR16(6, 28, 3);
const T2DColor red = COLOR16(28, 2, 2);
const T2DColor orange = COLOR16(27, 14, 2);
const T2DColor yellow = COLOR16(29, 29, 0);
const T2DColor purple = COLOR16(20, 0, 30);
const T2DColor cyan = COLOR16(3, 25, 25);
const T2DColor blue = COLOR16(2, 8, 26);
const T2DColor lightBlue = COLOR16(7, 10, 20);
const T2DColor darkBlue = COLOR16(5, 5, 15);
T2DColor _colors[8] = {darkGrey, green, red, orange, yellow, purple, cyan, blue};

T2DColor* GetCellColors()
{        
    return _colors;
}