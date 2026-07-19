#pragma once
#include "../Game.h"
#include "../utils/Math.h"
#include "../utils/Sound.h"
#include "../utils/FontWriter.h"
#include "Grid.h"
#include "Blocks.cpp"
#include <circle/bcmrandom.h>
#include <circle/2dgraphics.h>
#include <circle/usb/usbgamepad.h>
#include <circle/timer.h>
#include <circle/string.h>

class TetrisGame : public Game
{
public:
    TetrisGame(int screenWidth = 0, int screenHeight = 0);
    ~TetrisGame();
    void Update(CTimer *timer) override;
    void Draw(C2DGraphics *graphics) override;    
    void HandleInput(TGamePadState gamePadState) override;

    
    
private:
    void DrawUI(C2DGraphics *graphics);    
    void MoveBlockLeft();
    void MoveBlockRight();
    void MoveBlockDown();
    Block GetRandomBlock();
    bool IsBlockOutside();
    void RotateBlock();
    void LockBlock();
    bool BlockFits();
    void Reset();
    void UpdateScore(int linesCleared, int moveDownPoints);
    void CheckForHighScore();

    Grid grid;
    Block* blocks;
    Block currentBlock;
    Block nextBlock;

    CBcmRandomNumberGenerator rnd = CBcmRandomNumberGenerator();    
    CString scoreText;
    CString highscoreText;
    CString debugText;

    Sound* sndFlip = nullptr;
    Sound* sndSuccess = nullptr;
    FontWriter *writer = nullptr;



    int cellSize;
    int xoffset;
    int centerOfFreeSpace;
    int gameSpeed = 100;
    bool gameOver;
};