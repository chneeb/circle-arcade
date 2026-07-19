#include "FontWriter.h"

//#define SPACING 5

//Available fonts:
/*
 font_I-pixel-u_20x20.lmi
 font_I-pixel-u_40x40.lmi
 font_retropixel_20x20
 font_retropixel_40x40
*/


FontWriter::~FontWriter()
{
    delete font;
    ready = false;
}

FontWriter::FontWriter(const char *filename, Size dimensions)
{        
        ready = true;
        font = new Image(filename);
        fontsize = dimensions;
}



void FontWriter::Write(int x, int y, CString text, C2DGraphics *graphics, C2DGraphics::TTextAlign alignment, TRawPixel transparent)
{
    //assume 16 char width
    
    int source_x = 0;
    int source_y = 0;    
    int dx = x;
    if(alignment == C2DGraphics::AlignCenter)
        dx -= (this->CalculateStringWidth(text) / 2);
    if(alignment == C2DGraphics::AlignRight)
        dx -= this->CalculateStringWidth(text);
    int dy = y;    


    for(size_t i = 0; i < text.GetLength(); i++)
    {        
        //32-47 is the first available char (SPACE)
        //48-57 first digit (0-9)
        //65-90 caps aplha (A-Z)
        //97-122 small alpha (a-z)
        
        int index = (int)text[i];

        if(index < 32 || index > 122)
            continue;

        index -= 32;
        source_x = (index % 16) * fontsize.width;
        source_y = (index / 16) * fontsize.heigt;

        graphics->DrawImageRectTransparent(dx, dy, fontsize.width, fontsize.heigt, source_x, source_y,  font->getWidth(), font->getHeight(), font->getImageData16(), RawToColor(transparent));

        dx += fontsize.width;
    }
}


int FontWriter::CalculateStringWidth(CString text)
{
    int width = 0;

    for(size_t i = 0; i < text.GetLength(); i++)
    {            
        int index = (int)text[i];

        //Not a printable char so ignore
        if(index < 32 || index > 122)
            continue;

        width += fontsize.width;
    }
    return width;
}
