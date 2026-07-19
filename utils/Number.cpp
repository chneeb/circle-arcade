#include "Number.h"

Number::~Number()
{

}

Number::Number()
{
}

void Number::Draw(int x, int y, int number, C2DGraphics *graphics, ImageAlignment align)
{
    int tmp = number;
    int noOfDigits = 0;
    int digitWidth = digits[0].getWidth();
    int digitHeight = digits[0].getHeight();

    if(number == 0) {
        noOfDigits = 1;
    } else {    
        while(tmp > 0)
        {
            tmp = tmp / 10;
            noOfDigits++;
        }
    }

    int numberWidth = noOfDigits * digitWidth;
    int digitPosition = 1;

    if(number == 0)
    {
        int dx = x;
        if(align == Center)
            dx = dx - digitWidth/2;
        graphics->DrawImageTransparent(dx, y, digitWidth, digitHeight, digits[0].getImageData16(), BLACK_COLOR);
    } else {
        while (number > 0)
        {
            int digit = number % 10;
            int dx = x + (noOfDigits - digitPosition) * digitWidth;

            if(align == Center)
                dx = dx - (numberWidth/2);

            graphics->DrawImageTransparent(dx, y, digitWidth, digitHeight, digits[digit].getImageData16(), BLACK_COLOR);
            number = number / 10;
            digitPosition++;
        }    
    }
}


