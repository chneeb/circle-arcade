#pragma once
#include <circle/2dgraphics.h>
#include <circle/usb/usbgamepad.h>
#include <circle/sound/pwmsounddevice.h>
#include <circle/timer.h>

#include "utils/Text.h"
#include "utils/Color.h"




class Game
{
public:    
    Game(int width, int height);    
    virtual ~Game();
    virtual void Draw(C2DGraphics *graphics) {};
    virtual void Update(CTimer *timer) {};
    virtual void HandleInput(TGamePadState gamePadState) {};
    void setSoundDevice(CPWMSoundDevice *device);
    void SetActive(bool state);
    bool isActive() { return active; };

private:
    bool active = true;

protected:

    void SaveHighscoreToFile(int highscore, const char* file);
    int LoadHighscoreFromFile(const char* file);
    bool EventTriggered(double interval, int ticks);

    
    int score;
    int highscore;
    double lastUpdateTime = 0;
    int keyDelay = 0;
    bool running;

    int screenWidth;
    int screenHeight;
    
    CPWMSoundDevice *soundDevice = nullptr;
};