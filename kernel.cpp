//
// kernel.cpp
//
#include "kernel.h"
#include "utils/SimpleGamePadDefs.h"

#include "snake/SnakeGame.h"
#include "tetris/TetrisGame.h"
#include "pong/PongGame.h"
#include "invaders/InvadersGame.h"


#include <assert.h>

#define DRIVE		"SD:"
#define DEVICE_INDEX	1		// "upad1"

// Set to 1 to show what the attached gamepad actually reports, on the menu
// screen. Useful to find out how a pad sends its D-pad.
#define GAMEPAD_DEBUG	0

// The axes a gamepad reports its D-pad on. Pads without a mapping known to
// Circle do not necessarily use the first two: the USB SNES clone tested here
// uses 3 and 4. Enable GAMEPAD_DEBUG to see which axes a given pad moves.
#define DPAD_AXIS_X	3
#define DPAD_AXIS_Y	4

CKernel *CKernel ::s_pThis = 0;



CKernel::CKernel (void) :
#if USE_ST7789
  m_SPIMaster (ST7789_CLOCK_SPEED, ST7789_CPOL, ST7789_CPHA, ST7789_SPI_DEVICE),
  m_ST7789 (&m_SPIMaster, ST7789_DC_PIN, ST7789_RESET_PIN, ST7789_BACKLIGHT_PIN,
	    ST7789_WIDTH, ST7789_HEIGHT, ST7789_CPOL, ST7789_CPHA,
	    ST7789_CLOCK_SPEED, ST7789_CHIP_SELECT, ST7789_SWAP_COLOR_BYTES),
  m_2DGraphics(&m_ST7789),
#else
  m_2DGraphics(m_Options.GetWidth(), m_Options.GetHeight()),
#endif
  m_Timer(&m_Interrupt),
  m_USBHCI(&m_Interrupt, &m_Timer,TRUE),
  m_EMMC(&m_Interrupt, &m_Timer, &m_ActLED),
  m_SoundDevice (&m_Interrupt),
  m_pGamePad(0)
{
	s_pThis = this;
	m_ActLED.Blink (5);
}

CKernel::~CKernel (void)
{
	s_pThis = 0;
	delete background;
	delete btnInvaders;
	delete btnPong;
	delete btnSnake;
	delete btnTetris;
	delete activeGame;
	delete writer;
}

boolean CKernel::Initialize (void)
{
	boolean success = TRUE;

#if USE_ST7789
	if(success)
	{
		success = m_SPIMaster.Initialize ();
	}
	if(success)
	{
		success = m_ST7789.Initialize ();
	}
#if ST7789_ROTATE_180
	if(success)
	{
		RotateDisplay180 ();
	}
#endif
#endif
	if(success)
	{
		success = m_2DGraphics.Initialize ();
	}
	if(success)
	{
		success = m_Interrupt.Initialize();
	}
	if(success)
	{
		success = m_Timer.Initialize();
	}
	if(success)
	{
		success = m_USBHCI.Initialize();
	}
	if(success)
	{
		success = m_EMMC.Initialize();
	}

#if USE_GPIO_BUTTONS
	if(success)
	{
		InitializeButtons ();
	}
#endif

	return success;
}

TShutdownMode CKernel::Run (void)
{
#if USE_ST7789 && ST7789_TEST_PATTERN
	DrawTestPattern ();
	return ShutdownHalt;
#endif

	// Mount file system (fail check omitted)
	f_mount(&m_FileSystem, DRIVE, 1);

	//Load images 	
	background = new Image("/gfx/menu.lmi");
	btnPong = new Image("/gfx/pong.lmi");
	btnTetris = new Image("/gfx/tetris.lmi");
	btnSnake = new Image("/gfx/snake.lmi");
	btnInvaders = new Image("/gfx/invaders.lmi");
	sndMenu = new Sound("/audio/menu_loop.raw", 2, 16);

	writer = new FontWriter("/gfx/fonts/font_I-pixel-u_20x20_anti.lmi", {20,20});
	screenHeight = m_2DGraphics.GetHeight();
	screenWidth = m_2DGraphics.GetWidth();
	


	for (unsigned int cycle = 0; 1; cycle++)
	{
#if USE_GPIO_BUTTONS
		UpdateButtons ();
#else
		boolean bUpdated = m_USBHCI.UpdatePlugAndPlay ();

		if(m_pGamePad == 0)
		{
			if (!bUpdated) {
				continue;
			}
					
			m_pGamePad = (CUSBGamePadDevice *) m_DeviceNameService.GetDevice ("upad", DEVICE_INDEX, FALSE);
			
			if (m_pGamePad == 0) {
				m_2DGraphics.ClearScreen(BLACK_COLOR);
				writer->Write(screenWidth / 2, (screenHeight / 2) - writer->GetFontHeight(), "NO GAMEPAD DETECTED", &m_2DGraphics,C2DGraphics::AlignCenter );
//				m_2DGraphics.DrawText(screenWidth / 2, (screenHeight / 2) - 10, BLACK_COLOR, "NO GAMEPAD DETECTED", C2DGraphics::AlignCenter);
				writer->Write(screenWidth / 2, (screenHeight / 2) + 10, "PLEASE ATTACH ONE", &m_2DGraphics,C2DGraphics::AlignCenter );
//				m_2DGraphics.DrawText(screenWidth/ 2, (screenHeight / 2) + 10, BLACK_COLOR, "PLEASE ATTACH ONE", C2DGraphics::AlignCenter);
				m_2DGraphics.UpdateDisplay();
				continue;
			}
			
			m_pGamePad->RegisterRemovedHandler(GamePadRemovedHandler);
			
			// get initial state from gamepad and register status handler
			const TGamePadState *pState = m_pGamePad->GetInitialState();			
			assert (pState != 0);
			GamePadStatusHandler (DEVICE_INDEX-1, pState);
			
			m_pGamePad->RegisterStatusHandler (GamePadStatusHandler);
		}
#endif

		if(activeGame == nullptr || !activeGame->isActive())
		{
			MenuUpdate();
			continue;;
		}


		//Handle input
        activeGame->HandleInput(m_GamePadState);

		//Update
        activeGame->Update(&m_Timer);

        // Drawing        
        activeGame->Draw(&m_2DGraphics);

		//Update display
		m_2DGraphics.UpdateDisplay();
	}
		
	// Unmount file system (fail check omitted)
	f_mount (0, DRIVE, 0);

	return ShutdownHalt;

}

void CKernel::MenuUpdate()
{
	if(!muted)
		sndMenu->Play(&m_SoundDevice);
	
	//Background
	m_2DGraphics.ClearScreen(COLOR16(0,0,2));
	
	background->DrawAt(screenWidth/2, screenHeight/2, &m_2DGraphics, Center);		
	//List o' games
	
	btnSnake->DrawAt(screenWidth/2, screenHeight/2, &m_2DGraphics, TopCenter, 0x00, (selectedIndex == 0 ? COLOR16(0,0,31): 0x00));
	btnTetris->DrawAt(screenWidth/2, screenHeight/2+30, &m_2DGraphics, TopCenter, 0x00, (selectedIndex == 1 ? COLOR16(0,0,31): 0x00));
	btnPong->DrawAt(screenWidth/2, screenHeight/2+60, &m_2DGraphics, TopCenter, 0x00, (selectedIndex == 2 ? COLOR16(0,0,31): 0x00));
	btnInvaders->DrawAt(screenWidth/2, screenHeight/2+90, &m_2DGraphics, TopCenter, 0x00, (selectedIndex == 3 ? COLOR16(0,0,31): 0x00));

	//Selection dot
//	m_2DGraphics.DrawRect((screenWidth - 250 - 20) /2, (screenHeight/2) + (selectedIndex * 30)+ 5, 20, 20, BLUE_COLOR);
//	m_2DGraphics.DrawRect((screenWidth - 250)/2, (screenHeight/2) + (selectedIndex * 30) + 25, 250, 5, BLUE_COLOR);
		
//	m_2DGraphics.DrawText(10,10,WHITE_COLOR, debugText);

#if GAMEPAD_DEBUG
	{
		CString line;
		int y = 10;

		line.Format ("buttons %08X  naxes %d  nhats %d  nbuttons %d",
			     m_GamePadState.buttons, m_GamePadState.naxes,
			     m_GamePadState.nhats, m_GamePadState.nbuttons);
		writer->Write (10, y, line, &m_2DGraphics);
		y += 22;

		for (int i = 0; i < m_GamePadState.naxes && i < 8; i++)
		{
			line.Format ("axis %d = %d  [%d..%d]", i,
				     m_GamePadState.axes[i].value,
				     m_GamePadState.axes[i].minimum,
				     m_GamePadState.axes[i].maximum);
			writer->Write (10, y, line, &m_2DGraphics);
			y += 22;
		}

		for (int i = 0; i < m_GamePadState.nhats && i < 4; i++)
		{
			line.Format ("hat %d = %d", i, m_GamePadState.hats[i]);
			writer->Write (10, y, line, &m_2DGraphics);
			y += 22;
		}
	}
#endif

	m_2DGraphics.UpdateDisplay();

	if(keyDelay > 0)
	{
		keyDelay--;
		return;
	}


	
	if(code.GetSize() == 8) //Never going to happen for now
	{
		if(
			code[7] == GamePadButtonUp && 
			code[6] == GamePadButtonUp && 
			code[5] == GamePadButtonDown && 
			code[4] == GamePadButtonDown && 
			code[5] == GamePadButtonLeft && 
			code[4] == GamePadButtonRight && 
			code[3] == GamePadButtonLeft && 
			code[2] == GamePadButtonB && 
			code[1] == GamePadButtonA && 
			code[0] == GamePadButtonStart 
		)
		{
			writer->Write(screenWidth/2, screenHeight/2, "KONAMI STYLE!!!", &m_2DGraphics, C2DGraphics::AlignCenter );
		}
	}
	
	if(code.GetSize() == 3)
	{
		if(
			(code[2] & GamePadButtonLeft && code[1] & GamePadButtonLeft && code[0] & GamePadButtonTriangle )
			|| (code[2] & SimpleGamePadButtonA && code[1] & SimpleGamePadButtonA && code[0] & SimpleGamePadButtonSelect /*Same as GamePadButtonCircle*/)
		)
		{
			CString path;
			path.Format("/x/%02i.lmi", rnd.GetNumber() % 6);
			delete background;
        	background = new Image(path);
			code.clear();
		}
	}

	if( (m_GamePadState.buttons & GamePadButtonLeft) ||
	    (m_GamePadState.buttons & GamePadButtonRight) || 
		(m_GamePadState.buttons & GamePadButtonUp) || 
		(m_GamePadState.buttons & GamePadButtonDown) || 
		(m_GamePadState.buttons & GamePadButtonCross /*also equals SimpleGamePadStart*/) || 
		(m_GamePadState.buttons & GamePadButtonSquare) || 
		(m_GamePadState.buttons & GamePadButtonTriangle) ||
		(m_GamePadState.buttons & GamePadButtonCircle /*also equals SimpleGamePadSelect*/)   ||
		(m_GamePadState.buttons & SimpleGamePadButtonB)   ||
		(m_GamePadState.buttons & SimpleGamePadButtonA) )
	{
		keyDelay = 10;
		code.push(m_GamePadState.buttons);
	}

	
	
	//Up
	if (m_GamePadState.buttons & GamePadButtonUp) {
        if(selectedIndex > 0)
			selectedIndex--;
		keyDelay = 10;
//		code.push(m_GamePadState.buttons);
    }
	//Down
    if (m_GamePadState.buttons & GamePadButtonDown) {
        if(selectedIndex < 3)
            selectedIndex++;
		keyDelay = 10;
//		code.push(m_GamePadState.buttons);
    }

	//Mute
	if((m_GamePadState.buttons & GamePadButtonCircle) || (m_GamePadState.buttons & SimpleGamePadButtonSelect))
	{		
		if(m_SoundDevice.PlaybackActive()) 
			sndMenu->Stop(&m_SoundDevice);
		muted = !muted;
	}
	//Select
    if ((m_GamePadState.buttons & GamePadButtonCross /*same as SimpleGamePadButtonStart*/) || (m_GamePadState.buttons & SimpleGamePadButtonB)) {

		sndMenu->Stop(&m_SoundDevice);
		
		while(m_SoundDevice.PlaybackActive())
		{
			//Just wat for the audio sample to finish
		}

		switch (selectedIndex)
		{
		case 0:
			if(activeGame != nullptr) delete activeGame;					
			activeGame = new SnakeGame(m_2DGraphics.GetWidth(), m_2DGraphics.GetHeight());			
			break;
		case 1:
			if(activeGame != nullptr) delete activeGame;
			activeGame = new TetrisGame(m_2DGraphics.GetWidth(), m_2DGraphics.GetHeight());			
			break;
		case 2:
			if(activeGame != nullptr) delete activeGame;
			activeGame = new PongGame(m_2DGraphics.GetWidth(), m_2DGraphics.GetHeight());
			break;
		case 3:	
			if(activeGame != nullptr) delete activeGame;
			activeGame = new InvadersGame(m_2DGraphics.GetWidth(), m_2DGraphics.GetHeight());
			break;
		default:
			break;
		}

		if(activeGame != nullptr)
		{
			
			//m_SoundDevice = CPWMSoundDevice(&m_Interrupt);
			activeGame->setSoundDevice(&m_SoundDevice);
		}
		
		keyDelay = 10;
    }
}

#if USE_GPIO_BUTTONS

// How the GamePi20's buttons present themselves to the rest of the suite.
//
// Note that Circle's own constants and the SimpleGamePad ones in
// utils/SimpleGamePadDefs.h alias each other: GamePadButtonCircle and
// SimpleGamePadButtonSelect are both BIT(8), and GamePadButtonCross and
// SimpleGamePadButtonStart are both BIT(9). The mapping below is chosen so
// that every button lands on something the menu or a game already tests:
//
//   A       select in the menu, fire in Invaders, rotate in Tetris
//   X       restart after game over
//   START   back to the menu
//   SELECT  mute in the menu, and also back to the menu in a game
//
// B, Y and the two shoulder buttons are mapped to unused constants for now.
static const struct
{
	unsigned nPin;
	unsigned nButton;
}
s_ButtonMap[] =
{
	{ GPIO_BUTTON_UP,	GamePadButtonUp		},
	{ GPIO_BUTTON_DOWN,	GamePadButtonDown	},
	{ GPIO_BUTTON_LEFT,	GamePadButtonLeft	},
	{ GPIO_BUTTON_RIGHT,	GamePadButtonRight	},
	{ GPIO_BUTTON_A,	GamePadButtonCross	},
	{ GPIO_BUTTON_B,	GamePadButtonSquare	},
	{ GPIO_BUTTON_X,	GamePadButtonTriangle	},
	{ GPIO_BUTTON_Y,	GamePadButtonL3		},
	{ GPIO_BUTTON_TL,	GamePadButtonL1		},
	{ GPIO_BUTTON_TR,	GamePadButtonR1		},
	{ GPIO_BUTTON_SELECT,	GamePadButtonCircle	},
	{ GPIO_BUTTON_START,	GamePadButtonStart	}
};

void CKernel::InitializeButtons (void)
{
	static_assert (sizeof s_ButtonMap / sizeof s_ButtonMap[0] == GPIOButtonCount,
		       "s_ButtonMap and GPIOButtonCount disagree");

	for (unsigned i = 0; i < GPIOButtonCount; i++)
	{
		m_ButtonPins[i].AssignPin (s_ButtonMap[i].nPin);
		m_ButtonPins[i].SetMode (GPIOModeInputPullUp);
	}

	memset (&m_GamePadState, 0, sizeof m_GamePadState);
	m_GamePadState.nbuttons = GPIOButtonCount;
}

// The buttons pull their pin to ground, so a LOW level means pressed. Called
// once per frame; at the frame rate the panel runs at, that is slow enough
// that contact bounce never shows up.
void CKernel::UpdateButtons (void)
{
	unsigned nButtons = 0;

	for (unsigned i = 0; i < GPIOButtonCount; i++)
	{
		if (m_ButtonPins[i].Read () == LOW)
		{
			nButtons |= s_ButtonMap[i].nButton;
		}
	}

	m_GamePadState.buttons = nButtons;
}

#endif

#if USE_ST7789 && ST7789_ROTATE_180

// Turn the picture by 180 degrees in the panel itself.
//
// CST7789Display hardcodes MADCTL 0x70 (MX | MV | ML) in its init, and its
// SetRotation() only rotates in software. Inverting the MX and MY bits gives
// 0xB0 (MY | MV | ML), which is the same landscape mapping turned around, and
// costs nothing per frame.
//
// Command() and Data() are private in the driver, so the two bytes are sent
// here over the same SPI master and D/C pin the driver uses.
void CKernel::RotateDisplay180 (void)
{
	static const u8 MadctlCommand = 0x36;
	static const u8 MadctlRotated = 0xB0;

	CGPIOPin DCPin (ST7789_DC_PIN, GPIOModeOutput);

	m_SPIMaster.SetClock (ST7789_CLOCK_SPEED);
	m_SPIMaster.SetMode (ST7789_CPOL, ST7789_CPHA);

	DCPin.Write (LOW);
	m_SPIMaster.Write (ST7789_CHIP_SELECT, &MadctlCommand, sizeof MadctlCommand);

	DCPin.Write (HIGH);
	m_SPIMaster.Write (ST7789_CHIP_SELECT, &MadctlRotated, sizeof MadctlRotated);
}

#endif

#if USE_ST7789 && ST7789_TEST_PATTERN

// A static pattern to bring the panel up with, before any game code is
// involved. What it tells you:
//
//   - nothing at all      -> wiring, chip select, or the backlight pin
//   - bars in the wrong   -> ST7789_SWAP_COLOR_BYTES is wrong
//     colours
//   - markers in the      -> orientation is mirrored, adjust the MADCTL value
//     wrong corners          in CST7789Display::Initialize
//   - a shifted or        -> a GRAM offset is being applied that this 240x320
//     wrapped image          panel does not need
//
void CKernel::DrawTestPattern (void)
{
	unsigned nWidth  = m_2DGraphics.GetWidth ();
	unsigned nHeight = m_2DGraphics.GetHeight ();

	m_2DGraphics.ClearScreen (BLACK_COLOR);

	// Four vertical bars, to check the colour order.
	const T2DColor Bars[4] =
	{
		COLOR16 (31,  0,  0),		// red
		COLOR16 ( 0, 31,  0),		// green
		COLOR16 ( 0,  0, 31),		// blue
		COLOR16 (31, 31, 31)		// white
	};

	unsigned nBarWidth = nWidth / 4;
	for (unsigned i = 0; i < 4; i++)
	{
		m_2DGraphics.DrawRect (i * nBarWidth, 0, nBarWidth, nHeight, Bars[i]);
	}

	// A border, to check that the full panel is addressed and that no GRAM
	// offset is being applied. Three pixels wide, because one is hard to make
	// out at the very edge of the panel.
	for (unsigned i = 0; i < 3; i++)
	{
		m_2DGraphics.DrawRectOutline (i, i, nWidth - 2*i, nHeight - 2*i,
					      COLOR16 (31, 31, 0));
	}

	// Corner markers, to check the orientation. Yellow belongs top left,
	// magenta top right.
	m_2DGraphics.DrawRect (0, 0, 20, 20, COLOR16 (31, 31, 0));
	m_2DGraphics.DrawRect (nWidth - 10, 0, 10, 10, COLOR16 (31, 0, 31));

	m_2DGraphics.UpdateDisplay ();

	// Keep the picture up rather than returning into a halt with the panel
	// left in an undefined state.
	for (;;)
	{
		m_ActLED.Blink (1);
		m_Timer.MsDelay (1000);
	}
}

#endif

void CKernel::GamePadStatusHandler (unsigned nDeviceIndex, const TGamePadState *pState)
{
	if (nDeviceIndex != DEVICE_INDEX-1) {
		return;
	}

	assert (s_pThis != 0);
	assert (pState != 0);
	memcpy (&s_pThis->m_GamePadState, pState, sizeof *pState);
	NormalizeGamePadState (&s_pThis->m_GamePadState);
}

// Only gamepads with a mapping known to Circle report their D-pad as digital
// buttons. Cheap pads send it as a hat switch, or as an analog axis pair, and
// then the direction bits stay empty. Fill them in here, so that the menu and
// the games can just read buttons, whatever the pad turns out to be.
void CKernel::NormalizeGamePadState (TGamePadState *pState)
{
	// Hat switch: 0 is up, then clockwise in steps of 45 degrees. Any value
	// outside 0..7 means the hat is centered.
	if (pState->nhats >= 1)
	{
		static const unsigned HatToButtons[8] =
		{
			GamePadButtonUp,
			GamePadButtonUp   | GamePadButtonRight,
			GamePadButtonRight,
			GamePadButtonDown | GamePadButtonRight,
			GamePadButtonDown,
			GamePadButtonDown | GamePadButtonLeft,
			GamePadButtonLeft,
			GamePadButtonUp   | GamePadButtonLeft
		};

		int nHat = pState->hats[0];
		if (0 <= nHat && nHat < 8)
		{
			pState->buttons |= HatToButtons[nHat];
		}
	}

	// Analog axes: the outer quarter of the range counts as a direction. The
	// range is taken from the pad itself instead of assuming 0..255. Only the
	// configured axes are looked at, so that an unrelated axis resting at one
	// end of its range cannot read as a direction that is held down forever.
	if (pState->naxes > DPAD_AXIS_X)
	{
		pState->buttons |= AxisToButtons (pState, DPAD_AXIS_X,
						  GamePadButtonLeft, GamePadButtonRight);
	}

	if (pState->naxes > DPAD_AXIS_Y)
	{
		pState->buttons |= AxisToButtons (pState, DPAD_AXIS_Y,
						  GamePadButtonUp, GamePadButtonDown);
	}
}

unsigned CKernel::AxisToButtons (const TGamePadState *pState, unsigned nAxis,
				 unsigned nLowButton, unsigned nHighButton)
{
	int nMinimum = pState->axes[nAxis].minimum;
	int nMaximum = pState->axes[nAxis].maximum;
	int nValue   = pState->axes[nAxis].value;

	int nRange = nMaximum - nMinimum;
	if (nRange <= 0)
	{
		return 0;
	}

	if (nValue <= nMinimum + nRange/4)
	{
		return nLowButton;
	}

	if (nValue >= nMaximum - nRange/4)
	{
		return nHighButton;
	}

	return 0;
}

void CKernel::GamePadRemovedHandler (CDevice *pDevice, void *pContext)
{
	assert (s_pThis != 0);
	s_pThis->m_pGamePad = 0;
}

