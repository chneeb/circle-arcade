#include "InvadersGame.h"
#include "../utils/SimpleGamePadDefs.h"
#define Y_OFFSET 5
#define MIN_SPAWN_INTERVAL 450
#define SPAWN_INTERVAL 900

// Alien formation. The pitch is the original spacing; the margin is the same
// one MoveAliens turns the aliens around at, so the formation never starts
// outside the area it is allowed to move in.
#define ALIEN_SPRITE_WIDTH 44
#define ALIEN_SPRITE_HEIGHT 40
#define ALIEN_PITCH_X      55
#define ALIEN_PITCH_Y      55
#define ALIEN_MARGIN_X     25
#define ALIEN_MAX_COLUMNS  11
#define ALIEN_MAX_ROWS     5

// A 320x240 panel cannot fit the full-size furniture. The alien sprites are up
// to 44x40 and nothing scales them, so on a short screen there is only room for
// a two-row formation, and only if the obstacles are drawn smaller too. On a
// 640x480 screen nothing below changes anything.
#define COMPACT_LAYOUT_MAX_HEIGHT 360
#define ALIEN_PITCH_Y_COMPACT     (ALIEN_SPRITE_HEIGHT + 2)
#define OBSTACLE_SCALE            3
#define OBSTACLE_SCALE_COMPACT    2

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

bool InvadersGame::IsCompactLayout() const
{
    return screenHeight <= COMPACT_LAYOUT_MAX_HEIGHT;
}

void InvadersGame::CreateObstacles()
{

    int scale = IsCompactLayout() ? OBSTACLE_SCALE_COMPACT : OBSTACLE_SCALE;
    int obstacleWidth =  (sizeof(Obstacle::grid[0]) / sizeof(Obstacle::grid[0][0])) * scale;
    int obstacleHeight = (sizeof(Obstacle::grid) / sizeof(Obstacle::grid[0])) * scale;
    float gap = (screenWidth - (4 * obstacleWidth))/5;

    obstacleTop = spaceship->GetRect().y - obstacleHeight - 20;

    for(int i = 0; i < 4; i++) {
        float offsetX = (i + 1) * gap + i * obstacleWidth;
        obstacles.push_back(new Obstacle({offsetX, float(obstacleTop)}));
    }
}

void InvadersGame::CreateAliens()
{
    int offsetY = Y_OFFSET + writer->GetFontHeight() + mysteryship->GetHeight();

    // The alien sprites are a fixed size, up to 44x40, and nothing scales them,
    // so on a small screen the formation has to lose rows and columns rather
    // than shrink. Work out what actually fits: horizontally inside the margins
    // that MoveAliens turns the aliens around at, vertically between the score
    // line and the obstacles.
    //
    // A 640x480 screen gets 10 x 5, a 320x240 one 4 x 2. The latter is sparse
    // but playable; the aliens are simply too big for more to fit.
    //
    // 640x480 loses one column against the 11 it used to place. The old
    // formation was 594 px wide starting at x=45, so it reached x=639 - past
    // the screenWidth-25 line MoveAliens turns the aliens around at, which made
    // the formation drop a row on the very first frame. Ten columns start
    // inside the area they are allowed to move in.
    int usableWidth  = screenWidth - 2*ALIEN_MARGIN_X;
    int usableHeight = obstacleTop - offsetY;

    int pitchY = IsCompactLayout() ? ALIEN_PITCH_Y_COMPACT : ALIEN_PITCH_Y;

    int columns = usableWidth / ALIEN_PITCH_X;
    int rows    = usableHeight / pitchY;

    if(columns > ALIEN_MAX_COLUMNS) columns = ALIEN_MAX_COLUMNS;
    if(rows > ALIEN_MAX_ROWS)       rows = ALIEN_MAX_ROWS;
    if(columns < 1)                 columns = 1;
    if(rows < 1)                    rows = 1;

    int formationWidth = (columns - 1) * ALIEN_PITCH_X + ALIEN_SPRITE_WIDTH;
    int offsetX = (screenWidth - formationWidth) / 2;

    for(int row = 0; row < rows; row++) {
        for(int column = 0; column < columns; column++) {

            int alienType;
            if(row == 0) {
                alienType = 3;
            } else if (row == 1 || row == 2) {
                alienType = 2;
            } else {
                alienType = 1;
            }

            float x = offsetX + column * ALIEN_PITCH_X;
            float y = offsetY + row * pitchY;
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
        
        // Not getSize()-1: with one alien left that is a division by zero, and
        // it also meant the last alien in the array never fired. Reaching one
        // alien is common now that a small screen holds far fewer of them.
        int randomIndex = rnd.GetNumber() % aliens.getSize(); 
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



