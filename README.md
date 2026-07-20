# Arcade Suite for Raspberry Pi

The Arcade Suite consist of 4 classic retro games: Tetris, Space Invaders, Snake and Pong and a simple menu. The games are built using Circle - a C++ bare metal programming environment for the Raspberry Pi.
To compile and set up the suite, follow the steps below. Circle is included as a submodule, but requires some initial setup.

The games themselves are pretty straight forward and it should be fairy easy to expand or modify them. If you wish to change or add new adio or graphics you need to comply to a few constraints. The audio must be exported as raw (headerless),signed 16-bit PCM data using e.i. Audacity. The graphics are loaded as rgb565 LMI files (see https://github.com/general-ackbar/lmiencoder for pure MacOS version or https://github.com/general-ackbar/img2lmi for platform independent utility).

This version is optimized for the Raspberry Pi 1, using one USB-controller. Video output through either composite or HDMI and audio through jack.

## Prerequisites

- **Toolchain Requirements**:
  - For Raspberry Pi 1: [ARM1176JZF core with EABI support](#http://elinux.org/Rpi_Software#ARM)
  - For Raspberry Pi 2/3/4: [Cortex-A7, A53, or A72 support](#https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

   Please make sure to install it before proceeding.

## Setup Instructions

1. **Clone the Repository with Submodules**:

   Clone the repository and initialize the submodules recursively.

   ```bash
   git clone --recursive https://github.com/general-ackbar/circle-arcade.git

2. Build Circle Framework:

	Navigate to the Circle submodule directory and build it. For the Raspberry Pi 1, you don't need to alter the default settings.
	
	```bash
	cd arcade-suite/circle
	./configure
	make all
	```
	
   For more details and options, refer to the README file in the Circle submodule directory or the original Circle repository.

3. Compile the SDCard Addon:

	Navigate to the SDCard addon directory and compile it.

	```bash
	cd addon/SDCard
	make
	

## Build the Arcade Suite:

1.  Return to the root directory of the project and build the entire suite.

    ```bash
	cd ../../
	make
    ```
   This should generate a kernel.img file.

##  Final Steps

Format an SD card as FAT32 and copy the following into the root of it.

1. Copy kernel.img to the SD card.

2. Copy the necessary boot files to the SD card

    For the Raspberry Pi 1 and Zero, the required boot files are:
    * bootcode.bin
    * fixup.dat
    * start.elf

   These are not part of this repository. To download them, run `make` in
   the `circle/boot` directory.

3. Copy `circle/boot/config32.txt` to the SD card and rename it to `config.txt`.

4. Create a `cmdline.txt` file in the SD card root specifying the resolution:

	```
	width=640 height=480
	```

	Note that this goes in `cmdline.txt`, not in `config.txt`. These are Circle
	kernel options, and `config.txt` is the firmware's own configuration file,
	which ignores them silently. Without this the suite boots to a black screen,
	because the menu is laid out relative to a screen size that stays zero.

5. Copy the `gfx` and `audio` directories to the SD card, keeping their names.
   The games load their graphics and sounds from there at runtime.

6. Eject SD card, plug it in the RPI, attach a controller and have fun :)

The SD card should end up looking like this:

	bootcode.bin
	start.elf
	fixup.dat
	config.txt
	cmdline.txt
	kernel.img
	gfx/
	audio/

For later rebuilds only kernel.img needs to be copied again.

## Resources

[Circle Project](#https://github.com/rsta2/circle)



	