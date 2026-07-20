# circle-arcade — Claude Notes

## Project Overview

Four retro games (Tetris, Space Invaders, Snake, Pong) plus a menu, running bare
metal on a Raspberry Pi via [Circle](https://github.com/rsta2/circle). No OS.
Circle is a submodule, pinned to a release tag.

Targets the Pi 1 / Zero (`RASPPI=1`). Two hardware configurations, both built
from the same source and selected at compile time:

| | Frame buffer | Waveshare GamePi20 |
|---|---|---|
| Picture | HDMI or composite | ST7789 panel over SPI |
| Input | USB gamepad | the board's buttons on GPIO |
| Sound | headphone jack | onboard speaker / jack, GPIO 18 |
| Selected by | `USE_ST7789 0`, `USE_GPIO_BUTTONS 0` | `USE_ST7789 1`, `USE_GPIO_BUTTONS 1` |

The switches live in `DisplayConfig.h` and `InputConfig.h`. The GamePi20 build
runs games, menu, input and sound on the device; what is left is mostly that the
assets are still cut for 640x480 — see [Porting to the GamePi20](#porting-to-the-gamepi20).

## Key Files

- `main.cpp` — entry point, halts or reboots on the return value of `CKernel::Run()`
- `kernel.cpp` / `kernel.h` — hardware init, SD mount, asset loading, menu, gamepad handling
- `Game.h` — base class for all games: `Draw(C2DGraphics*)`, `Update(CTimer*)`, `HandleInput(TGamePadState)`
- `DisplayConfig.h` / `InputConfig.h` — which display and input the build uses, plus the GamePi20 pin numbers
- `ST7789DMADisplay.{h,cpp}` — the ST7789 driver in use, sending frames over DMA
- `configure-gamepi20.sh` — configures and builds Circle with the options sound needs
- `utils/Color.h` — colour helpers for the Circle Step49+ display API (see below)
- `utils/Image.cpp` — LMI asset loader and blitter, including the tinting path used by the menu
- `utils/FontWriter.cpp` — bitmap font rendering from an LMI sheet
- `snake/`, `tetris/`, `pong/`, `invaders/` — one directory per game
- `gfx/`, `audio/` — runtime assets, loaded from the SD card, not compiled in

`assets/` holds two placeholder headers and is **not** what ships. The real
assets are in `gfx/` and `audio/`.

## Build

```bash
./configure-gamepi20.sh      # configures and builds Circle and its addons
make                         # produces kernel.img
```

**Use the script, not a bare `./configure`.** Sound needs three options that
live in `circle/include/circle/sysconfig.h`, inside the submodule, so they are
passed to Circle's `configure` with `-d` instead of being edited in. `configure`
writes them to `circle/Config.mk`, which Circle **gitignores** — the script is
the only record of them. Reconfiguring Circle by hand loses audio, and the only
symptom is silence. See [Audio](#audio) for why the three belong together.

Without sound, the equivalent by hand is:

```bash
cd circle && ./configure -r 1 && ./makeall -j$(nproc)
cd addon/SDCard && make
cd ../fatfs && make
cd ../display && make        # needed for the ST7789 panel
cd ../../.. && make
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

On the GamePi20 build the resolution in `cmdline.txt` is ignored — the panel is
a fixed 320x240 and the size is read from it. The file is still worth keeping on
the card so the same card boots a frame buffer build.

## Circle Version and the Colour API

Pinned to **Step51**. Step49 is the minimum, because that is where
`C2DGraphics` gained a `CDisplay *` constructor — which is what lets it render
to the GamePi20's ST7789 panel instead of the VideoCore framebuffer, with no
change to any game or menu code.

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
(`GetDepth()` = 16) are 16bpp, so that cast is correct for either display.

## USB Gamepad Handling

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

Working on hardware: panel, buttons and sound are all running on the device.
Configured through `DisplayConfig.h`, `InputConfig.h` and
`configure-gamepi20.sh`.

Bring-up was deliberately staged — Circle upgrade on HDMI first, then the panel
with a test pattern, then input, then sound — so that a failure at each step
could only have one cause. `ST7789_TEST_PATTERN` in `DisplayConfig.h` still
draws that pattern, which is the quickest way to re-check wiring, orientation
and colour order after touching anything on the display side.

Still open:

- **Assets are cut for 640x480.** The menu background is 640x480 and is dropped
  whole on a 320x240 panel — `DrawImage` bails out when the image does not fit
  (`lib/2dgraphics.cpp:435`), so it does not even clip. The games lay out from
  `screenWidth`/`screenHeight` and mostly cope; Invaders needed real work (see
  below).
- **Rendering at 640x480 and halving into the panel** was considered as a way to
  avoid re-cutting anything: a `CDisplay` wrapper that reports 640x480 and
  decimates 2:1 in `SetArea`. It costs 4x the rendering plus the decimation, and
  dropping every other pixel destroys the anti-aliased fonts. Not done.
- **HDMI and panel at the same time** would be a second `CDisplay` wrapper
  forwarding `SetArea` to both. Cheap, because Circle owns the source buffer —
  unlike fbcp on Linux, there is no readback. Both outputs must agree on size
  and colour model.

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
- The panel is mounted upside down, so the picture is turned 180 degrees by
  inverting the MX and MY bits of MADCTL — `0x70` becomes `0xB0`. That is free;
  `SetRotation(180)` would have been correct arithmetically but only rotates in
  software. `Command()` and `Data()` are private in the driver, so
  `CKernel::RotateDisplay180()` sends the two bytes over the same SPI master and
  D/C pin, which avoids patching the submodule.

The SPI clock is 62.5 MHz, the fastest divisor a 250 MHz core allows, and well
above what the ST7789VW is specified for. Tearing, flicker or intermittent noise
means step down to 41.7 MHz — a marginal clock looks like a wiring fault.

#### ST7789DMADisplay

`ST7789DMADisplay.{h,cpp}` is the driver actually in use. It is a `CDisplay` of
our own rather than a subclass, because `CST7789Display` keeps `SendData`,
`SetWindow`, `Command` and `Data` private — there is nothing for a subclass to
reuse, and patching the pinned submodule is worse. Writing our own also folds
MADCTL into the panel's init instead of sending it again afterwards.

`SetArea` copies the frame into a buffer of its own, starts the transfer and
returns. **It waits for the previous frame at the start of the next call, not at
the end of its own** — that ordering is what buys the overlap. The copy is what
makes it safe: `C2DGraphics` has a single buffer, so without it the games would
draw into pixels still being clocked out.

Three things about Circle's DMA SPI that the headers do not make obvious:

- `StartWriteRead` asserts `nCount <= 0xFFFF`, but a frame is 153,600 bytes. It
  goes out in chunks of 61,440 — 96 whole rows, and 4-byte aligned as
  `spimasterdma.h` requires — chained through the completion routine.
- `StartWriteRead` also asserts a non-null read buffer. That is not waste: with
  no RX DMA the receive FIFO is never drained and the controller stalls.
- Chip select is driven **by hand** (`ChipSelectNone` plus a plain output on
  BCM 8), so it can stay low across every chunk of one frame. Letting the
  peripheral toggle CE0 per transfer would break the RAMWR stream at each chunk
  boundary.

The panel is blacked out during init, before the backlight comes on — its memory
holds power-up garbage otherwise, and lighting it first shows a screenful of
noise until the first frame lands.

`ST7789_USE_DMA 0` falls back on Circle's polled driver, which is how to tell a
DMA problem apart from anything else.

### Input

`InputConfig.h` holds the pin numbers and `USE_GPIO_BUTTONS`. The buttons are
read once per frame in `CKernel::UpdateButtons()` and presented as a
`TGamePadState`, so nothing downstream knows whether input came from the board
or from a USB pad. The two are a compile-time choice, not simultaneous: with
`USE_GPIO_BUTTONS` the USB gamepad code is compiled out entirely.

The mapping has to work around Circle's constants and the SimpleGamePad ones in
`utils/SimpleGamePadDefs.h` aliasing each other — `GamePadButtonCircle` and
`SimpleGamePadButtonSelect` are both `BIT(8)`, `Cross` and
`SimpleGamePadButtonStart` both `BIT(9)`. Every button is mapped onto something
the menu or a game already tests: A selects and fires, X restarts, START and
SELECT go back to the menu. B, Y, TL and TR read correctly but nothing listens.

No debounce is needed — one sample per frame is far slower than contact bounce.

### Audio

Working, mono, on GPIO 18, which feeds both the earphone jack and the onboard
speaker. Enabled by three options passed through `configure-gamepi20.sh`:

```
USE_PWM_AUDIO_ON_ZERO
USE_GPIO18_FOR_LEFT_PWM_ON_ZERO
USE_GPIO19_FOR_RIGHT_PWM_ON_ZERO
```

**The three belong together.** `USE_PWM_AUDIO_ON_ZERO` alone puts PWM audio on
**GPIO 12 and 13 — this board's Up and Right buttons** (`lib/machineinfo.cpp`,
`GetGPIOPin`), i.e. outputs wired to switches that short to ground when pressed.
Circle's own comment in `sysconfig.h` warns this may destroy the pins. The other
two move it to GPIO 18, and to GPIO 19 which is not connected here — hence mono.

To check a build before flashing it, disassemble `GetGPIOPin` and confirm no 12
or 13 appears:

```bash
arm-none-eabi-ar x circle/lib/libcircle.a machineinfo.o
arm-none-eabi-objdump -d machineinfo.o \
	--disassemble=_ZNK12CMachineInfo10GetGPIOPinE15TGPIOVirtualPin | grep '#1[23]\b'
```

The settings end up in `circle/Config.mk`, which is gitignored, so they are lost
by any bare `./configure` — with silence as the only symptom.

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

The frame goes out over DMA (`ST7789_USE_DMA`), so the CPU is free during the
transfer. Circle's own `CST7789Display::SendData` uses `CSPIMaster::Write`,
which is polled — the CPU spins for the whole 19.7 ms, most of the budget.
`ST7789DMADisplay` replaces it; see below.

A Pi Zero 2 W is worth considering: Circle treats it as `RASPPI=3`, it has a
quad Cortex-A53, and it keeps the GamePi20's form factor and power budget.

### Game layout at 320x240

`screenWidth`/`screenHeight` are read from the display (`m_2DGraphics.GetWidth()`),
not from `cmdline.txt`. They used to come from `m_Options`, which on the panel
would have been zero — laying the whole menu out off-screen.

Invaders needed real work, and is worth reading before adapting the others. Its
alien formation was hardcoded to 11 columns at a 55 px pitch, 594 px wide: on a
320 px screen `offsetX` came out at -115, so aliens sat off both edges and
`MoveAliens` saw its left and right turn-around conditions true at the same time,
marching the formation down every frame. Rows and columns are now derived from
the space actually available, and a short screen also gets a tighter row pitch
and 2x rather than 3x obstacles, which is what makes room for a second row.
640x480 gets 10 x 5, 320x240 gets 4 x 2.

640x480 lost a column in the process. The old 11 reached x=639, past the
`screenWidth-25` line `MoveAliens` turns around at, so the formation dropped a
row on its first frame there too.

Eight aliens is sparse. The sprites are up to 44x40 and nothing scales them, so
that is the ceiling at this resolution without smaller art.

## Assets

Graphics are RGB565 LMI files. Audio is raw headerless signed 16-bit PCM. See
the README for the encoder tools.

`kernel.cpp` loads `/x/%02i.lmi` for a Konami-code easter egg, but no `x/`
directory exists in the repo — it fails harmlessly if triggered.
