#include "InvadersGame.h"
#include "../utils/SimpleGamePadDefs.h"
#define Y_OFFSET 5
#define MIN_SPAWN_INTERVAL 450 
#define SPAWN_INTERVAL 900

InvadersGame::InvadersGame(int width, int height) : Game(width, height)
{
    writer = new FontWriter("/gfx/fonts/font_retropixel_20x20.lmi", {20,20}); 
    playerLaser = new Sound("/audio/invaders_player_laser.raw", 2, 16);
    alienLaser  = new Sound("/audio/invaders_alien_laser.raw", 2, 16);
    explosion = new Sound("/audio/invaders_explosion.raw", 2, 16);   
    InitGame();

}

InvadersGame::~InvadersGame() {    
    delete spaceship;
    delete spaceshipImage;
    delete mysteryship;
    delete writer;
}

void InvadersGame::Update(CTimer *timer) {
    
    
    fireTicks = timer->GetTicks();
    
    if(running) {

        double currentTime = timer->GetTicks();        
        if(currentTime - timeLastSpawn > mysteryShipSpawnInterval) {
            mysteryship->Spawn(Y_OFFSET + writer->GetFontHeight());
            timeLastSpawn = timer->GetTicks();
            mysteryShipSpawnInterval = MIN_SPAWN_INTERVAL + (rnd.GetNumber() % SPAWN_INTERVAL);
        }

        for(int i = 0; i < spaceship->lasers.getSize(); i++) {
            spaceship->lasers[i]->Update();
        }

        
        MoveAliens();

        AlienShootLaser(timer);

        for(int i = 0; i < alienLasers.getSize(); i++) {            
            alienLasers[i]->Update();
        }

        DeleteInactiveLasers();

        mysteryship->Update();

        CheckForCollisions();
    
    } else {
        if(false){
            Reset();
            InitGame();
        }
    }

    
}

void InvadersGame::Draw(C2DGraphics *graphics) {
    graphics->ClearScreen(BLACK_COLOR);
    

    spaceship->Draw(graphics);

    
    for(int i = 0; i < spaceship->lasers.getSize(); i++) {
        spaceship->lasers[i]->Draw(graphics);
    }

    for(int i = 0; i < aliens.getSize(); i++) {
        aliens[i]->Draw(graphics);
    }


    for(int i = 0; i < obstacles.getSize(); i++) {
        obstacles[i]->Draw(graphics);
    }

    for(int i = 0; i < alienLasers.getSize(); i++) {
        alienLasers[i]->Draw(graphics);
    }

    mysteryship->Draw(graphics);

    DrawUI(graphics);
}

void InvadersGame::DrawUI(C2DGraphics *graphics)
{
    if(running){                
        writer->Write(screenWidth/2, screenHeight - writer->GetFontHeight(), lvlText, graphics, C2DGraphics::AlignCenter);
    } else {
        writer->Write(screenWidth/2, screenHeight/ 2, "GAME OVER", graphics, C2DGraphics::AlignCenter);
    }
    
    //lives
    float x = 30.0;
    for(int i = 0; i < lives; i ++) {
        spaceshipImage->DrawAt(x, screenHeight-spaceshipImage->getHeight()-1, graphics);
        x += 30;
    }

    scoreText.Format("Score: %05i", score);
    writer->Write(20, Y_OFFSET, scoreText, graphics); //, 0x00, C2DGraphics::AlignLeft);
    
    writer->Write(screenWidth-20, Y_OFFSET, highscoreText, graphics, C2DGraphics::AlignRight);

//    debugText.Format("Screen: %i x %i", screenWidth, screenHeight);
//    graphics->DrawText(10,10, WHITE_COLOR, debugText);
}

void InvadersGame::HandleInput(TGamePadState gamePadState) {
        
    if(keyDelay > 0)
        keyDelay--;


    if ((gamePadState.buttons & GamePadButtonLeft)){        
        spaceship->MoveLeft();       
    }
    if ((gamePadState.buttons & GamePadButtonRight)){        
        spaceship->MoveRight();
    }
    if (keyDelay == 0 && ( (gamePadState.buttons & GamePadButtonCross) || (gamePadState.buttons & SimpleGamePadButtonB) ) ) {
        if(spaceship->FireLaser(fireTicks))
            playerLaser->Play(soundDevice);
        
        keyDelay = 10;
    }

    if (!running && ((gamePadState.buttons & GamePadButtonTriangle) || (gamePadState.buttons & SimpleGamePadButtonA ))) {
       Reset();     
    }

    //On modern controllers "Start" and "Circle" will exit
    if ((gamePadState.buttons & GamePadButtonStart) || (gamePadState.buttons & SimpleGamePadButtonSelect /*also equals GamePadCircle*/ )) {
        this->SetActive(false);
    }
}

void InvadersGame::DeleteInactiveLasers()
{
    int index = 0;
    while(index < spaceship->lasers.getSize())
    {
        if(!spaceship->lasers[index]->active)
        {
            spaceship->lasers.erase(index);
        } else
            index++;
    }

    index = 0;
    while(index < alienLasers.getSize())
    {
        if(!alienLasers[index]->active)
        {
            alienLasers.erase(index);
        } else
            index++;
    }
}

void InvadersGame::CreateObstacles()
{
    
    int obstacleWidth =  (sizeof(Obstacle::grid[0]) / sizeof(Obstacle::grid[0][0])) * 3;
    int obstacleHeight = (sizeof(Obstacle::grid) / sizeof(Obstacle::grid[0])) * 3;
    float gap = (screenWidth - (4 * obstacleWidth))/5;

    for(int i = 0; i < 4; i++) {
        float offsetX = (i + 1) * gap + i * obstacleWidth;
        obstacles.push_back(new Obstacle({offsetX, float(spaceship->GetRect().y - obstacleHeight - 20)}));
    }    
}

void InvadersGame::CreateAliens()
{
    int offsetX = (screenWidth - (10*55)) / 2;      //center 10 columns, each alien is 50px
    int offsetY = Y_OFFSET + writer->GetFontHeight() + mysteryship->GetHeight();
    for(int row = 0; row < (screenHeight > 480 ? 5 : 4); row++) {               //Tiresome hack for small screens
        for(int column = 0; column < 11; column++) {

            int alienType;
            if(row == 0) {
                alienType = 3;
            } else if (row == 1 || row == 2) {
                alienType = 2;
            } else {
                alienType = 1;
            }

            float x = offsetX + column * 55;
            float y = offsetY + row * 55;
            aliens.push_back(new Alien(alienType, {x, y}));
        }
    }
}

void InvadersGame::MoveAliens() {
    for(int i = 0; i < aliens.getSize(); i ++) {
        Alien *alien = aliens[i];
        if(alien->position.x + alien->GetRect().width > screenWidth - 25) {
            aliensDirection = -1;
            MoveDownAliens(alienSpeed);
        }
        if(alien->position.x < 25) {
            aliensDirection = 1;
            MoveDownAliens(alienSpeed);
        }

        alien->Update(aliensDirection);
    }

    
}

void InvadersGame::MoveDownAliens(int distance)
{
    
    for(int i = 0; i < aliens.getSize(); i ++) {
        aliens[i]->position.y += distance;
    }
}

void InvadersGame::AlienShootLaser(CTimer *timer)
{
    
    double currentTime = timer->GetTicks();
    if((currentTime - timeLastAlienFired) >= alienLaserShootInterval && !aliens.isEmpty()) {
        
        int randomIndex = rnd.GetNumber() % (aliens.getSize()-1); 
//        debugText.Format("Laser triggered from alien %i", randomIndex);
        Alien *alien = aliens[randomIndex];
        
        alienLasers.push_back(new Laser({alien->position.x + alien->GetRect().width/2, 
                                    alien->position.y + alien->GetRect().height}, 6, screenHeight));
        alienLaser->Play(soundDevice);
        timeLastAlienFired = timer->GetTicks();
    }
}

void InvadersGame::CheckForCollisions()
{
    
    //Spaceship Lasers
    for(int laser_counter = 0; laser_counter < spaceship->lasers.getSize(); laser_counter ++) {
        
        Laser *laser = spaceship->lasers[laser_counter];
        
        int alien_counter = 0;
        while(alien_counter < aliens.getSize()) {
            
            Alien *alien = aliens[alien_counter];

            if(Math::CheckRectangleCollision(alien->GetRect(), laser->GetRect()))
            {
                explosion->Play(soundDevice);
                if(alien->GetType() == 1) {
                    score += 100 + (level-1)*25;
                } else if (alien->GetType() == 2) {
                    score += 200 + (level-1)*25;
                } else if(alien->GetType() == 3) {
                    score += 300 + (level-1)*25;
                }
                checkForHighscore();

                aliens.erase(alien_counter);
                laser->active = false;
            } else
                alien_counter++;
        }
        
        if(aliens.getSize() == 0){
            NextLevel();
        }

        for(int obstacle_counter = 0; obstacle_counter < obstacles.getSize(); obstacle_counter ++) {
            Obstacle *obstacle = obstacles[obstacle_counter];

            if(!Math::CheckRectangleCollision(laser->GetRect(), obstacle->GetRect()))
                continue;
            
            int block_counter = 0;
            while(block_counter < obstacle->blocks.getSize())
            {
                if(Math::CheckRectangleCollision(obstacle->blocks[block_counter].getRect(), laser->GetRect())){
                    obstacle->blocks.erase(block_counter);
                    laser->active = false;  
                } else
                    block_counter++;
            }
        }

        if(Math::CheckRectangleCollision(mysteryship->getRect(), laser->GetRect())){
            mysteryship->alive = false;
            spaceship->lasers[laser_counter]->active = false;
            score += 500 + (level-1)*45;
            checkForHighscore();
            explosion->Play(soundDevice);
        }
    }

    //Alien Lasers 
    for(int laser_counter = 0; laser_counter < alienLasers.getSize(); laser_counter ++) {
        Laser *laser = alienLasers[laser_counter];

        if(Math::CheckRectangleCollision(laser->GetRect(), spaceship->GetRect())){
            laser->active = false;
            lives--;
            explosion->Play(soundDevice);
            if(lives == 0) {
                GameOver();
            }
        }

        for(int obstacle_counter = 0; obstacle_counter < obstacles.getSize(); obstacle_counter ++) {
            Obstacle *obstacle = obstacles[obstacle_counter];

            if(!Math::CheckRectangleCollision(laser->GetRect(), obstacle->GetRect()))
                continue;

            int block_counter = 0;
            while(block_counter < obstacle->blocks.getSize()) {
                
                if(Math::CheckRectangleCollision(obstacle->blocks[block_counter].getRect(), laser->GetRect())){
                    obstacle->blocks.erase(block_counter);
                    laser->active = false;                
                } else {
                    block_counter++;
                }
            }
        }
    }
    
    //Alien Collision with Obstacle or player
    for(int alien_counter = 0; alien_counter < aliens.getSize(); alien_counter ++) {
        Alien *alien = aliens[alien_counter];

        
//        if(alien->position.y > (screenHeight - 210))
//            continue;

        for(int obstacle_counter = 0; obstacle_counter < obstacles.getSize(); obstacle_counter ++) {
            Obstacle *obstacle = obstacles[obstacle_counter];

            if(!Math::CheckRectangleCollision(alien->GetRect(), obstacle->GetRect()))
                continue;

            int block_counter = 0;
            while(block_counter < obstacle->blocks.getSize()) {
                if(Math::CheckRectangleCollision(obstacle->blocks[block_counter].getRect(), alien->GetRect())) {
                    obstacle->blocks.erase(block_counter);
                } else {
                    block_counter++;
                }
            }
        }

        if(Math::CheckRectangleCollision(alien->GetRect(), spaceship->GetRect())) {
            GameOver();
        }
    }
}

void InvadersGame::GameOver()
{
    running = false;
}

void InvadersGame::InitGame()
{
    
    highscore = LoadHighscoreFromFile("invaders.txt");
    highscoreText.Format("Higscore: %05i", highscore);

    mysteryship = new MysteryShip({0.0,0.0, float(screenWidth), float(screenHeight)});    
    mysteryShipSpawnInterval = MIN_SPAWN_INTERVAL + (rnd.GetNumber() % SPAWN_INTERVAL);
    timeLastSpawn = 0.0;

    spaceship = new Spaceship({0.0,0.0, float(screenWidth), float(screenHeight)});

    CreateObstacles();
    CreateAliens();
    aliensDirection = 1;
    timeLastAlienFired = 0.0;
    alienSpeed = 3;



    spaceshipImage = new Image("/gfx/invaders/spaceship_icon.lmi");
    running = true;
    lives = 3;
    score = 0;
    level = 1;

    lvlText.Format("LEVEL %02i", level);
}

void InvadersGame::checkForHighscore()
{
    if(score > highscore) {
        highscore = score;
        highscoreText.Format("Higscore: %05i", highscore);
        SaveHighscoreToFile(highscore, "invaders.txt");
    }
}

void InvadersGame::NextLevel()
{
    level++;    
    aliens.clear();
    alienLasers.clear();
    timeLastSpawn = 0.0;
    alienSpeed +=2;
    if(alienLaserShootInterval > 10)
        alienLaserShootInterval -= 10;

    CreateAliens();
    lvlText.Format("LEVEL %02i", level);
}

void InvadersGame::Reset() {
        
    spaceship->Reset();
    aliens.clear();
    alienLasers.clear();
    obstacles.clear();

    highscore = LoadHighscoreFromFile("invaders.txt");
    score = 0;
    level = 1;
    lives = 3;
    alienSpeed = 3;
    alienLaserShootInterval = 200.0;
    
    
    timeLastSpawn = 0.0;

    CreateAliens();
    CreateObstacles();

    running = true;
}



