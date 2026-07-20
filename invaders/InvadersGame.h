
#pragma once
#include <circle/2dgraphics.h>
#include <circle/bcmrandom.h>
#include <circle/usb/usbgamepad.h>
#include <circle/timer.h>
#include "../utils/Math.h"
#include "../utils/Array.h"
#include "../utils/Sound.h"
#include "../utils/FontWriter.h"

#include "../Game.h"
#include "SpaceShip.h"
#include "Obstacle.h"
#include "Alien.h"
#include "MysteryShip.h"

class InvadersGame : public Game {
    public:
        InvadersGame(int width, int height);
        ~InvadersGame();
        void Draw(C2DGraphics *graphics) override;
        void Update(CTimer *timer) override;
        void HandleInput(TGamePadState gamePadState) override;

        
    private:
        void DrawUI(C2DGraphics *graphics);
        void DeleteInactiveLasers();
        void CreateObstacles();
        void CreateAliens();
        bool IsCompactLayout() const;
        void MoveAliens();
        void MoveDownAliens(int distance); 
        void AlienShootLaser(CTimer *timer);
        void CheckForCollisions();
        void GameOver();
        void Reset();
        void NextLevel();
        void InitGame();
        void checkForHighscore();

        //Alien alien;
        Spaceship *spaceship;
        MysteryShip *mysteryship;
        Array<Alien*> aliens = Array<Alien*>();
        Array<Obstacle*> obstacles = Array<Obstacle*>();
        int obstacleTop = 0;    // set by CreateObstacles, used to place the aliens
        Array<Laser*> alienLasers = Array<Laser*>();

        int aliensDirection;
        //constexpr static float alienLaserShootInterval = 100.0;
        float alienLaserShootInterval = 200.0;
        float timeLastAlienFired;
        int alienSpeed = 3;
        
        
        float mysteryShipSpawnInterval;
        float timeLastSpawn;
        

        CBcmRandomNumberGenerator rnd = CBcmRandomNumberGenerator();

        bool running;
        int lives;
        int level;
        float fireTicks = 0;
        
        Image* spaceshipImage;
        FontWriter *writer = nullptr;
        CString scoreText;
        CString highscoreText;
        CString debugText;
        CString lvlText;

        Sound *playerLaser;
        Sound *alienLaser;
        Sound *explosion;
};