
#ifndef FontWriter_h
#define FontWriter_h
#pragma once

#include <fatfs/ff.h>
#include <circle/string.h>
#include <circle/2dgraphics.h>
#include "Color.h"
#include "Image.h"
#include "Math.h"




class FontWriter{


    
private:
    Image *font;
    bool ready;
    Size fontsize;

public:	
    ~FontWriter();
    FontWriter(const char* filename, Size dimensions);      
    void Write(int x, int y, CString string, C2DGraphics *graphics, C2DGraphics::TTextAlign alignment = C2DGraphics::AlignLeft, TRawPixel transparent = 0x00);    
    int CalculateStringWidth(CString string);
    int GetFontHeight() {return fontsize.heigt; };
    int GetFontWidth() {return fontsize.width; };
};

#endif 
