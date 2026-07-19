#include "Colors.h"

const TScreenColor darkGrey = COLOR16(15, 15, 15);
const TScreenColor green = COLOR16(6, 28, 3);
const TScreenColor red = COLOR16(28, 2, 2);
const TScreenColor orange = COLOR16(27, 14, 2);
const TScreenColor yellow = COLOR16(29, 29, 0);
const TScreenColor purple = COLOR16(20, 0, 30);
const TScreenColor cyan = COLOR16(3, 25, 25);
const TScreenColor blue = COLOR16(2, 8, 26);
const TScreenColor lightBlue = COLOR16(7, 10, 20);
const TScreenColor darkBlue = COLOR16(5, 5, 15);
TScreenColor _colors[8] = {darkGrey, green, red, orange, yellow, purple, cyan, blue};

TScreenColor* GetCellColors()
{        
    return _colors;
}