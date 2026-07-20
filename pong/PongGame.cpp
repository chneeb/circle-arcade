#include "PongGame.h"
#include "../utils/SimpleGamePadDefs.h"
#define CPU_DELAY 15

PongGame::PongGame(int width, int height) : Game(width, height)
{
    screenWidth = width;
    screenHeight = height;

    court = Court(0, 50, screenWidth, screenHeight-50);
    ball = Ball(10);
    ball.setCourt(&court);
    ball.setPosition(court.getWidth() / 2, court.getHeight() / 2);
    ball.setSpeed(5,5);

    player = Paddle();
    player.setCourt(&court);
    player.setSize(15, court.getHeight() / 5);
    player.setPosition(court.getWidth() - player.getWidth() - 10, court.getHeight() / 2 - player.getHeight() / 2);
    player.setSpeed(6);

    cpu = Cpu();
    cpu.setCourt(&court);
    cpu.setSize(15, court.getHeight() / 5);
    cpu.setPosition(10, (court.getHeight()  / 2) - (cpu.getHeight() / 2));
    cpu.setSpeed(5);

    sndPaddle = new Sound("/audio/pong_bounce.raw", 2 , 16);

    SetActive(true);
}

PongGame::~PongGame()
{    
    delete sndPaddle;
}

void PongGame::Draw(C2DGraphics *graphics)
{

    court.Draw(graphics);
    ball.Draw(graphics);
    cpu.Draw(graphics);
    player.Draw(graphics);
    DrawUI(graphics);

}

void PongGame::Update(CTimer *timer)
{
    if(!running)
        return;

    ball.Move();
    cpu.Update(ball.getY());

    if (player.CheckCollisionWithBall(ball)) {
        ball.FlipHorisontalDirection();
        sndPaddle->Play(soundDevice);
    }

    if (cpu.CheckCollisionWithBall(ball)) {
        ball.FlipHorisontalDirection();
        cpu.SetDelay(CPU_DELAY); //random?
        //cpu.SetSpeed( random ?)
        sndPaddle->Play(soundDevice);
    }



}

void PongGame::HandleInput(TGamePadState gamePadState)
{
    if(keyDelay > 0)
        keyDelay--;

    if ((gamePadState.buttons & GamePadButtonTriangle) || (gamePadState.buttons & SimpleGamePadButtonA ) ){        
        if(keyDelay == 0)
        {
            running = !running;
            keyDelay = 10;
        }
    }
    if ((gamePadState.buttons & GamePadButtonDown)){        
        player.MoveDown();        
    }
    if ((gamePadState.buttons & GamePadButtonUp)){        
        player.MoveUp();
    }

    if ((gamePadState.buttons & GamePadButtonStart) || (gamePadState.buttons & SimpleGamePadButtonSelect /*also equals GamePadCircle*/ )) {
        this->SetActive(false);
    }
}

void PongGame::DrawUI(C2DGraphics *graphics)
{
    number.Draw(screenWidth / 4 - 20, 10, court.getLeftScore(), graphics, ImageAlignment::Center);
    number.Draw(3*screenWidth / 4 - 20, 10, court.getRightScore(), graphics, ImageAlignment::Center);
}
