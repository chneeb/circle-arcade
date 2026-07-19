#include "SnakeGame.h"
#include "../utils/SimpleGamePadDefs.h"

SnakeGame::SnakeGame(int width, int height) : Game(width, height)
{
    imgLogo = new Image("/gfx/retro_snake.lmi");
    sndApple = new Sound("/audio/snake_apple.raw", 2, 16);
    sndCrash = new Sound("/audio/snake_crash.raw", 2, 16);
    writer = new FontWriter("/gfx/fonts/font_retropixel_20x20.lmi", {20,20});

    score = 0;
    highscore = LoadHighscoreFromFile("/snake.txt");
    highscoreText.Format("%i", highscore);

    screenHeight = height - 80;    
    cellCount = 25;
    cellSize = screenHeight / cellCount;
    offset = cellSize *2;

    snake = Snake(cellSize);
    food = Food(cellSize, cellCount);
    food.SetRandomPos(snake.body);
    SetActive(true);
}

SnakeGame::~SnakeGame()
{
    delete imgLogo;
    delete writer;
}


void SnakeGame::Draw(C2DGraphics *graphics)
{
    graphics->ClearScreen(COLOR16(0, 25, 0));
    DrawUI(graphics);
    food.Draw(graphics);
    snake.Draw(graphics);
    
}

void SnakeGame::DrawUI(C2DGraphics *graphics)
{
    int levelWidth = cellSize*cellCount;
    int centerOfFreeSpace = offset+levelWidth + (screenWidth - levelWidth) / 2;
    
    graphics->DrawRectOutline(offset-5, offset-5, levelWidth+10, levelWidth+10, COLOR16(31,31,31));        
    graphics->DrawRectOutline(offset-3, offset-3, levelWidth+6, levelWidth+6, COLOR16(31,31,31));        
    graphics->DrawImageTransparent(offset + levelWidth + (screenWidth - levelWidth - imgLogo->getWidth()) / 2, screenHeight/2, imgLogo->getWidth(), imgLogo->getHeight(), imgLogo->getImageData16(), BLACK_COLOR);
    
    writer->Write(centerOfFreeSpace, 50, "Highscore", graphics, C2DGraphics::AlignCenter);

    writer->Write(centerOfFreeSpace, 90, highscoreText, graphics, C2DGraphics::AlignCenter);

    number.Draw(offset + levelWidth + (screenWidth - levelWidth) / 2, screenHeight/2 + imgLogo->getHeight() + 20, score, graphics, ImageAlignment::Center);

}

void SnakeGame::Update(CTimer *timer)
{
    if(EventTriggered(gameSpeed, timer->GetTicks()))
    {
        allowMove = true;        
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
            
        }
    }
}

void SnakeGame::CheckCollisionWithFood()
{
    if (Math::Vector2DEquals(snake.body.peek_front(), food.position))
    {
        food.SetRandomPos(snake.body);
        snake.addSegment = true;
        sndApple->Play(soundDevice);
        score++;
        checkForHighscore();
    }
}

void SnakeGame::CheckCollisionWithEdges()
{
    if (snake.body.peek_front().x == cellCount || snake.body.peek_front().x == -1){
        GameOver();
    }
    if (snake.body.peek_front().y == cellCount || snake.body.peek_front().y == -1){
        GameOver();
    }
}

void SnakeGame::CheckCollisionWithTail()
{
    Deque headlessBody = snake.body;
    headlessBody.pop_front();
        if (headlessBody.Contains(snake.body.peek_front())){
        GameOver();
    }
}

void SnakeGame::checkForHighscore()
{
    if(score > highscore) {
        highscore = score;
        highscoreText.Format("%i", highscore);
        SaveHighscoreToFile(highscore, "/snake.txt");
    }
}

// 0x100 = select, 0x200 = start, 0x01 = B, 0x02 = A
void SnakeGame::HandleInput(TGamePadState gamePadState)
{    
    if (((gamePadState.buttons & GamePadButtonUp) || (gamePadState.axes[1].value == 0)) && snake.direction.y != 1 && allowMove) {
            snake.direction = {0, -1};
            running = true;
            allowMove = false;
    }
    if (((gamePadState.buttons & GamePadButtonDown) || (gamePadState.axes[1].value == 255)) && snake.direction.y != -1 && allowMove) {
            snake.direction = {0, 1};
            running = true;
            allowMove = false;
    }
    if (((gamePadState.buttons & GamePadButtonLeft) || (gamePadState.axes[0].value == 0))&& snake.direction.x != 1 && allowMove) {
            snake.direction = {-1, 0};
            running = true;
            allowMove = false;
    }
    if (((gamePadState.buttons & GamePadButtonRight) || (gamePadState.axes[0].value == 255)) && snake.direction.x != -1 && allowMove) {
            snake.direction = {1, 0};
            running = true;
            allowMove = false;
    }

    if ((gamePadState.buttons & GamePadButtonStart) || (gamePadState.buttons & SimpleGamePadButtonSelect /*also equals GamePadCircle*/ )) {
            snake.Reset();
            running = false;
            score = 0;
            this->SetActive(false);
    }
}

void SnakeGame::GameOver()
{
    snake.Reset();
    food.SetRandomPos(snake.body);
    running = false;
    score = 0;
}