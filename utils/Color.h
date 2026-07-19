//
//  Color.h
//
//  Colour helpers for the Circle Step49+ display API.
//
//  Circle used to expose a single RGB565 type, TScreenColor, for both the
//  colours passed to C2DGraphics and the pixels stored in image data. Step49
//  split the two: drawing takes a logical RGB888 T2DColor, while image buffers
//  stay raw and are interpreted using the display's colour model. Circle still
//  ships COLOR16 in <circle/screen.h>, but it yields an RGB565 value, so
//  passing it to a drawing call now compiles and silently draws wrong colours.
//
//  This header keeps the two roles apart:
//    - T2DColor   (from Circle) for anything handed to C2DGraphics
//    - TRawPixel  for RGB565 pixels in LMI assets and the frame buffer
//
#pragma once

#include <circle/2dgraphics.h>
#include <circle/types.h>

// A raw RGB565 pixel, as stored in the LMI assets and in the 16bpp frame
// buffer behind C2DGraphics::GetBuffer().
typedef u16 TRawPixel;

// Logical colour from 0-31 components. Circle's own COLOR16 took a 0-31 range
// for all three channels (see the comment in <circle/screen.h>), so existing
// call sites keep working unchanged - they just produce a T2DColor now.
#undef COLOR16
#define COLOR16(red, green, blue) COLOR2D (((red)   & 0x1F) << 3, \
					   ((green) & 0x1F) << 3, \
					   ((blue)  & 0x1F) << 3)

// The named colours the games use, matching Circle's pre-Step49 values.
#undef BLACK_COLOR
#undef WHITE_COLOR
#undef BLUE_COLOR
#define BLACK_COLOR	COLOR2D (0, 0, 0)
#define WHITE_COLOR	COLOR2D (170, 170, 170)
#define BLUE_COLOR	COLOR2D (0, 0, 170)

// Convert a raw pixel to a logical colour. Image data is standard RGB565 with
// a 6-bit green channel, unlike Circle's COLOR16, which only fills the upper
// five bits of green.
inline T2DColor RawToColor (TRawPixel nPixel)
{
	return COLOR2D (((nPixel >> 11) & 0x1F) << 3,
			((nPixel >>  5) & 0x3F) << 2,
			( nPixel        & 0x1F) << 3);
}
