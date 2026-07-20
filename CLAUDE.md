# circle-arcade — Claude Notes

## Project Overview

Four retro games (Tetris, Space Invaders, Snake, Pong) plus a menu, running bare
metal on a Raspberry Pi via [Circle](https://github.com/rsta2/circle). No OS.
Circle is a submodule, pinned to a release tag.

Currently targets the Pi 1 / Zero (`RASPPI=1`) with HDMI or composite output, a
USB gamepad, and audio on the headphone jack. Work is under way to port it to
the Waveshare GamePi20 handheld — see [Porting to the GamePi20](#porting-to-the-gamepi20).

## Key Files

- `main.cpp` — entry point, halts or reboots on the return value of `CKernel::Run()`
- `kernel.cpp` / `kernel.h` — hardware init, SD mount, asset loading, menu, gamepad handling
- `Game.h` — base class for all games: `Draw(C2DGraphics*)`, `Update(CTimer*)`, `HandleInput(TGamePadState)`
- `utils/Color.h` — colour helpers for the Circle Step49+ display API (see below)
- `utils/Image.cpp` — LMI asset loader and blitter, including the tinting path used by the menu
- `utils/FontWriter.cpp` — bitmap font rendering from an LMI sheet
- `snake/`, `tetris/`, `pong/`, `invaders/` — one directory per game
- `gfx/`, `audio/` — runtime assets, loaded from the SD card, not compiled in

`assets/` holds two placeholder headers and is **not** what ships. The real
assets are in `gfx/` and `audio/`.

## Build

```bash
cd circle && ./configure -r 1 && ./makeall -j$(nproc)
cd addon/SDCard && make
cd ../fatfs && make
cd ../../.. && make          # produces kernel.img
```

`-r 1` covers the Pi 1, Zero and Zero W (all ARM1176JZF-S). The Pi 3 and
Zero 2 W are `-r 3` and produce `kernel8-32.img`, a **different filename** —
the firmware picks the kernel by name, so a stale `kernel.img` next to it will
be booted instead.

Toolchain: `arm-none-eabi-gcc` (Debian package `gcc-arm-none-eabi`). This is
always a cross-compile; bare metal means there is no OS on the Pi to build on.

The source was originally developed on macOS, where the filesystem is
case-insensitive. Keep `#include` casing exact or the build breaks on Linux.

## SD Card Layout

```
bootcode.bin    start.elf    fixup.dat      <- firmware, from `make` in circle/boot
config.txt                                  <- copy of circle/boot/config32.txt
cmdline.txt                                 <- width=640 height=480
kernel.img
gfx/    audio/
```

Two things that are easy to get wrong and both look like a display failure:

- **`width=`/`height=` go in `cmdline.txt`, not `config.txt`.** They are Circle
  kernel options. `config.txt` is the firmware's own file and ignores them
  silently. Without them `CKernelOptions::GetWidth()` returns 0, so
  `screenWidth`/`screenHeight` are 0 and the whole menu is laid out at
  coordinate 0 — it draws off-screen and you get a black screen on a Pi that is
  otherwise running fine.
- **The games load `gfx/` and `audio/` from the card at runtime** (`kernel.cpp`,
  `Run()`). Copying only `kernel.img` boots to a broken menu.

For later rebuilds only `kernel.img` needs recopying.

## Circle Version and the Colour API

Pinned to **Step51**. Step49 is the minimum, because that is where
`C2DGraphics` gained a `CDisplay *` constructor — which is what will let it
render to the GamePi20's ST7789 panel instead of the VideoCore framebuffer.

Step49 also split the old RGB565 `TScreenColor` into two distinct roles, and
both are used here:

| Role | Type | Where |
|---|---|---|
| Logical colour passed to `C2DGraphics` | `T2DColor` (RGB888) | tetris cell colours, all `DrawRect`/`ClearScreen`/`DrawText` calls |
| Raw pixel data | `TRawPixel` (u16, RGB565) | LMI assets, `Image::DrawTinted`, font glyphs |

`utils/Color.h` keeps them apart. It redefines `COLOR16` to produce a
`T2DColor`, so the existing 0-31-per-channel call sites are unchanged, and
provides `RawToColor()` for the few places a raw pixel has to become a logical
colour.

**Circle still ships its own `COLOR16` in `<circle/screen.h>`, and it still
returns an RGB565 value.** Passing that to a drawing call compiles fine and
silently draws the wrong colour. `utils/Color.h` `#undef`s it first. If colours
ever look subtly wrong, check that the file in question includes `utils/Color.h`.

`C2DGraphics::GetBuffer()` returns `void *` since Step49 and needs a cast to
`TRawPixel *`. Both the framebuffer path (`DEPTH` = 16) and the ST7789 path
(`GetDepth()` = 16) are 16bpp, so that cast stays correct after the TFT switch.

## Gamepad Handling

Circle only fills in the standard direction buttons for pads whose mapping it
knows (`GamePadPropertyIsKnown`). Cheap HID pads report their D-pad as a hat
switch or as analog axes, and then `GamePadButtonUp` and friends stay empty.

`CKernel::NormalizeGamePadState()` synthesizes those bits once, where the pad
state arrives, so the menu and all four games can just read `buttons`.

**The D-pad axis indices are pad-specific** and set at the top of `kernel.cpp`:

```c
#define DPAD_AXIS_X	3
#define DPAD_AXIS_Y	4
```

The USB SNES clone tested here uses axes 3 and 4, not the first two. Set
`GAMEPAD_DEBUG` to 1 in `kernel.cpp` to print the raw `TGamePadState` — button
mask, axis count, per-axis value and range, hat values — on the menu screen.
That is the fastest way to work out a new pad. If left/right and up/down come
out swapped, flip the two defines.

Normalization deliberately looks **only** at the two configured axes. An
earlier version tested `axes[0]`/`axes[1]` against a hardcoded 0 and 255 at
every input site; on a pad that does not populate those entries they stay 0,
and 0 was the test for Up and Left, so both read as permanently pressed. That
pinned the menu at the first entry and forced a constant direction in-game.

A Pi Zero has no USB-A port — the pad needs an OTG adapter on the *data* micro
USB socket (the middle one; the outer one is power only).

## Porting to the GamePi20

Not started. The suite currently runs on HDMI + USB gamepad, which was
deliberately validated first so that display and input problems can be told
apart from Circle-upgrade problems.

### Hardware (Waveshare GamePi20, BCM numbering)

| Function | BCM |
|---|---|
| LCD MOSI / SCK / CS | 10 / 11 / 8 (SPI0, CE0) |
| LCD D/C / RESET / backlight | 25 / 27 / 24 |
| A / B / X / Y | 23 / 4 / 22 / 17 |
| TL / TR | 5 / 6 |
| Up / Down / Left / Right | 12 / 20 / 21 / 13 |
| Select / Start | 16 / 26 |
| Earphone jack | 18 (mono) |

Buttons are active low and need `GPIOModeInputPullUp`. MISO (BCM 9) is not
connected, which is fine — the panel is write-only.

### Display

ST7789VW, **240x320 native** used as 320x240 landscape. Circle's driver is
`circle/addon/display/st7789display.{h,cpp}`, built with `make` in that
directory.

- Being a true 240x320 panel, it needs **no GRAM offset** — unlike the common
  240x240 variant, which needs an 80-row offset.
- Circle's init sends MADCTL `0x70` (MX|MV|ML). MV swaps X/Y, so the panel
  already comes up in landscape.
- **Do not use `SetRotation()`.** `RotX`/`RotY` index against `m_nWidth` and
  `m_nHeight` interchangeably, which is only coherent for a square display, and
  any non-zero rotation makes `SetArea` software-rotate every frame through an
  intermediate buffer. Declare 320x240 in the constructor and leave rotation 0,
  which is both correct and the only path that does a single bulk transfer.
- Pass `bSwapColorBytes = FALSE`. It selects the colour model
  (`CDisplay (bSwapColorBytes ? RGB565_BE : RGB565)`), and the LMI assets are
  plain little-endian RGB565. Getting it wrong makes either the images or every
  drawn colour look wrong, but not both.

### Audio

Circle's `USE_PWM_AUDIO_ON_ZERO` in `circle/include/circle/sysconfig.h` defaults
PWM audio to **GPIO 12 and 13 — which are this board's Up and Right buttons**.
Also define `USE_GPIO18_FOR_LEFT_PWM_ON_ZERO` and
`USE_GPIO19_FOR_RIGHT_PWM_ON_ZERO`; BCM 19 is not connected here, so audio ends
up mono on BCM 18, which is where the earphone jack is.

### Performance

A full 320x240x16bpp frame is 153,600 bytes. Circle derives the SPI divisor
from the measured core clock, so on a Pi 1 / Zero (250 MHz core) the usable
rates are 62.5 / 41.7 / 31.25 MHz.

| SPI clock | Full frame | Max fps |
|---|---|---|
| 62.5 MHz | 19.7 ms | 50.9 |
| 41.7 MHz | 29.5 ms | 34.0 |

**60 fps full-screen is not achievable over this SPI link at any clock**, before
any CPU work. Fine for these games; a hard constraint for anything emulator-like,
where the only way to 60 fps is a 256-wide pillarboxed frame (122,880 bytes,
63.6 fps at 62.5 MHz — about 94% bus utilization) rather than scaling to fill
the width.

`CST7789Display::SendData` uses `CSPIMaster::Write`, which is **polled and
blocking** — the CPU spins for the whole transfer. Circle ships `CSPIMasterDMA`,
and `SetArea` already takes a completion routine, but the ST7789 driver does not
use it. Anything needing CPU time during a frame has to address this first.

A Pi Zero 2 W is worth considering: Circle treats it as `RASPPI=3`, it has a
quad Cortex-A53, and it keeps the GamePi20's form factor and power budget.

### Known landmine

`screenWidth`/`screenHeight` come from `m_Options` (i.e. `cmdline.txt`). On the
TFT there is no firmware framebuffer to fall back on, so these should be read
from the display object instead — `m_2DGraphics.GetWidth()` — which also makes
the black-screen failure above impossible.

## Assets

Graphics are RGB565 LMI files. Audio is raw headerless signed 16-bit PCM. See
the README for the encoder tools.

`kernel.cpp` loads `/x/%02i.lmi` for a Konami-code easter egg, but no `x/`
directory exists in the repo — it fails harmlessly if triggered.
