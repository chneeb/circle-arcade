//
//  DisplayConfig.h
//
//  Where the suite sends its picture.
//
//  By default it renders to the frame buffer, which is the HDMI or composite
//  output, at the resolution given by width=/height= in cmdline.txt.
//
//  With USE_ST7789 it drives the ST7789 panel of the Waveshare GamePi20 over
//  SPI instead. The pin numbers below are that board's; they are SoC (BCM)
//  numbers, not header positions.
//
#pragma once

#include <display/st7789display.h>

// Set to 1 to render to the ST7789 panel instead of the frame buffer.
#define USE_ST7789		1

// Set to 1 to draw a static test pattern and stop, instead of running the
// games. Use this when bringing up the panel: it tells apart wiring problems,
// a wrong orientation and a wrong colour order, before any game code is
// involved. Ignored unless USE_ST7789 is 1.
#define ST7789_TEST_PATTERN	1

// The panel is a 240x320 ST7789VW driven in landscape. Circle's init sends
// MADCTL 0x70, whose MV bit swaps X and Y, so the panel comes up as 320x240
// and no GRAM offset is needed. Leave the rotation alone - CST7789Display
// rotates in software and its RotX/RotY are only coherent for a square panel.
#define ST7789_WIDTH		320
#define ST7789_HEIGHT		240

// GamePi20 wiring. MISO (BCM 9) is not connected, which is fine, as the panel
// is only ever written to.
#define ST7789_DC_PIN		25
#define ST7789_RESET_PIN	27
#define ST7789_BACKLIGHT_PIN	24

#define ST7789_SPI_DEVICE	0	// SPI0: MOSI 10, SCK 11
#define ST7789_CHIP_SELECT	0	// CE0 = BCM 8

// CPOL 0 is right because chip select is wired. Circle's own sample uses CPOL 1
// only because it leaves CS unconnected.
#define ST7789_CPOL		0
#define ST7789_CPHA		0

// Circle derives the SPI divisor from the measured core clock, so on a 250 MHz
// core the reachable rates are 62.5, 41.7 and 31.25 MHz. Start slow: bring the
// panel up first, then raise this. A full frame is 153,600 bytes, so the clock
// sets the frame rate ceiling directly - 62.5 MHz is about 51 fps.
#define ST7789_CLOCK_SPEED	31250000

// The GamePi20 has the panel mounted upside down, so the picture has to be
// turned by 180 degrees. This is done in the panel, by inverting the MX and MY
// bits of MADCTL, which costs nothing.
//
// Do not use CST7789Display::SetRotation() for this. It is only a software
// rotation: with a non-zero rotation, SetArea copies every pixel of every
// frame through an intermediate buffer, which at 76,800 pixels a frame is
// exactly the work this display cannot afford.
#define ST7789_ROTATE_180	1

// FALSE selects the RGB565 colour model rather than RGB565_BE. The LMI assets
// are plain little-endian RGB565, and C2DGraphics converts logical colours
// through the same model, so both have to agree. If images look right but
// drawn colours do not, or the other way round, this is the flag.
#define ST7789_SWAP_COLOR_BYTES	FALSE
