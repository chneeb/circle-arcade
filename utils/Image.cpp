//
//
//  Created by Jonatan Yde on 06/12/2017.
//  Copyright © 2017 Codeninja. All rights reserved.
//

#include "Image.h"
#include "noimg.h"


Image::Image(const char *filename){
   
    
    FIL myfile;
    if( f_open(&myfile, filename, FA_READ) == FR_OK)
    {
        UINT btr;
        
        uint8_t header[9];
        f_read(&myfile, header, 9, &btr);
        
        
        headerSize = (int)header[4];
        width = bytes_to_u16(header[5], header[6]);
        height = bytes_to_u16(header[7], header[8]);
        bitsPerPixel = (int)header[3];
        
        
        FSIZE_t len = f_size(&myfile);
        len = len - headerSize;

        uint8_t buffer[len];
        
        f_lseek(&myfile, headerSize);
        f_read(&myfile, buffer, len, &btr );
        f_close(&myfile);


        int bytes_length = width * height; //same as: len / (bitsPerPixel / 8)
        data = new uint16_t[bytes_length];
        

        int currentIndex = 0;
        for (size_t i = 0; i < len; i+=2)
        {
            uint16_t rgb565 = ((uint16_t)buffer[i] << 8) | buffer[i+1];          
            data[currentIndex] = rgb565;
            currentIndex++;
        }
    } else {
        data = no_img;
        width = 50;
        height = 50;
        bitsPerPixel = 3;
        headerSize = 0;
    }


}

Image::Image()
{
    data = no_img;
    width = 50;
    height = 50;
    bitsPerPixel = 3;
    headerSize = 0;
}

Image::Image(const Image& other) {
    width = other.width;
    height = other.height;
    if (other.data != nullptr) {
        data = new uint16_t[width * height];
        for (size_t i = 0; i < width * height; ++i) {
            data[i] = other.data[i];
        }
        copy = true;
    } else {
        data = nullptr;
    }
}

Image::~Image()
{
    delete[] data;
    data = nullptr;
}

void Image::DrawAt(int x, int y, C2DGraphics *graphics, ImageAlignment align, TRawPixel transparency, TRawPixel tint)
{
    int dx = x, dy = y;
    switch (align)
    {
    case TopLeft:      
        //default, do nothing 
        break;
    case CenterLeft:        
        dy = y - height/2;
        break;
    case BottomLeft:     
        dy = y+height;   
        break;
    case Center:      
        dx = x - width/2;  
        dy = y - height/2;
        break;
    case TopCenter:      
        dx = x - width/2;  
        break;
    default:
        break;
    }
    if(transparency != 0xff)
    {
        graphics->DrawImageTransparent(dx, dy, width, height, data, RawToColor(transparency));

        if(tint > 0x00)
        {
            DrawTinted(graphics, dx, dy, width, height, 0, 0, width, height, data, transparency, tint);
        }
    }
    else
        graphics->DrawImage(dx, dy, width, height, data);
    
    /*
    if(copy)
        graphics->DrawText(dx + width/2, dy, WHITE_COLOR, "COPY");
    */
}


void Image::DrawTinted(C2DGraphics *gfx,  unsigned nX, unsigned nY, unsigned nWidth, unsigned nHeight, unsigned nSourceX, unsigned nSourceY, unsigned nSourceWidth, unsigned nSourceHeight, TRawPixel *PixelBuffer, TRawPixel TransparentColor, TRawPixel tint)
{
    for(unsigned i=0; i<nHeight; i++)
	{
		for(unsigned j=0; j<nWidth; j++)
		{
			TRawPixel sourcePixel = PixelBuffer[(nSourceY + i) * nSourceWidth + j + nSourceX];
			if(sourcePixel != TransparentColor)
			{                
                if(sourcePixel > tint) 
                    sourcePixel -= tint;
                else
                    sourcePixel = 0x00;

				((TRawPixel *) gfx->GetBuffer())[(nY + i) * gfx->GetWidth() + j + nX] = sourcePixel;
			}
		}
	}
}


uint16_t* Image::getImageData16() { return data; }
int Image::getWidth() {return width; }
int Image::getHeight() {return height; }
int Image::getBitsPerColor() {return bitsPerPixel; }

