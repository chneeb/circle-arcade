//
// kernel.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2021  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
//#include <circle/screen.h>
#include <circle/2dgraphics.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
//#include <circle/logger.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/sound/pwmsounddevice.h>
#include <circle/usb/usbgamepad.h>
#include <circle/bcmrandom.h>
#include <SDCard/emmc.h>
#include <fatfs/ff.h>
#include <circle/types.h>
#include <circle/util.h>
#include <circle/spimaster.h>
#include <circle/gpiopin.h>
#include <display/st7789display.h>

#include "DisplayConfig.h"
#include "InputConfig.h"
#include "utils/Color.h"
#include "Game.h"
#include "utils/Image.h"
#include "utils/Sound.h"
#include "utils/FixedList.h"
#include "utils/FontWriter.h"

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot	
};

class CKernel
{
private:


public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);
	TShutdownMode Run (void);

private:
	void MenuUpdate();
#if USE_ST7789 && ST7789_ROTATE_180 && !ST7789_USE_DMA
	void RotateDisplay180();
#endif
#if USE_ST7789 && ST7789_TEST_PATTERN
	void DrawTestPattern();
#endif
	static void GamePadStatusHandler (unsigned nDeviceIndex, const TGamePadState *pState);
	static void GamePadRemovedHandler (CDevice *pDevice, void *pContext);
	static void NormalizeGamePadState (TGamePadState *pState);
#if USE_GPIO_BUTTONS
	void InitializeButtons();
	void UpdateButtons();
#endif
	static unsigned AxisToButtons (const TGamePadState *pState, unsigned nAxis,
				       unsigned nLowButton, unsigned nHighButton);


	// do not change this order
	CActLED				m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
#if USE_ST7789
#if ST7789_USE_DMA
	CST7789DMADisplay	m_ST7789;
#else
	CSPIMaster			m_SPIMaster;
	CST7789Display		m_ST7789;
#endif
#endif
	C2DGraphics			m_2DGraphics;
	CTimer				m_Timer;


	CUSBHCIDevice		m_USBHCI;
	CEMMCDevice			m_EMMC;
	CPWMSoundDevice		m_SoundDevice;
	FATFS				m_FileSystem;

	CUSBGamePadDevice * volatile m_pGamePad;
	TGamePadState	   	m_GamePadState;
#if USE_GPIO_BUTTONS
	static const unsigned GPIOButtonCount = 12;
	CGPIOPin			m_ButtonPins[GPIOButtonCount];
#endif
	static CKernel		*s_pThis;

	int 				screenWidth;
	int 				screenHeight;
	int 				selectedIndex = 0;
	int 				keyDelay = 0;
	Game*				activeGame = nullptr;

	CString 			debugText;
	CString				bgName;
	Image*				background = nullptr;
	Image*				btnPong = nullptr;
	Image*				btnSnake = nullptr;
	Image*				btnTetris = nullptr;
	Image*				btnInvaders = nullptr;
	Sound*				sndMenu = nullptr;
	FontWriter*			writer = nullptr;

	bool 				muted = false;
	FixedList<int>		code = FixedList<int>(3);
	CBcmRandomNumberGenerator rnd = CBcmRandomNumberGenerator();

/*
	int timeSince = 0;
	int counter = 0;
*/
};

#endif
