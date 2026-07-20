#
# Makefile
#

CIRCLEHOME = circle

OBJS	= main.o kernel.o Game.o 
OBJS	+= utils/Math.o utils/Deque.o utils/Image.o utils/Text.o utils/Number.o utils/Sound.o utils/FontWriter.o
OBJS 	+= snake/Snake.o snake/Food.o snake/SnakeGame.o 
OBJS 	+= tetris/Block.o tetris/Blocks.o tetris/Colors.o tetris/Grid.o tetris/Position.o tetris/TetrisGame.o 
OBJS	+= pong/Ball.o pong/Court.o pong/Cpu.o pong/Paddle.o pong/PongGame.o
OBJS	+= invaders/Alien.o invaders/ObstacleBlock.o invaders/Laser.o invaders/MysteryShip.o invaders/Obstacle.o invaders/SpaceShip.o invaders/InvadersGame.o

LIBS	= $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	  $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/addon/display/libdisplay.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/sound/libsound.a \
	  $(CIRCLEHOME)/lib/libcircle.a


include $(CIRCLEHOME)/app/Rules.mk

-include $(DEPS)

SUBDIR_ROOTS := snake pong tetris utils invaders
DIRS := . $(shell find $(SUBDIR_ROOTS) -type d)
GARBAGE_PATTERNS := *.o *.d
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

clean_all:
	@rm -rf $(GARBAGE)
