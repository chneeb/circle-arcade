#include "TetrisGame.h"
#include "../utils/SimpleGamePadDefs.h"

TetrisGame::TetrisGame(int width, int weight) : Game(width, weight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight - SPACING;
    cellSize = this->screenHeight / ROWS;
    
    grid = Grid(cellSize);
    xoffset = grid.GetWidth();
    centerOfFreeSpace = xoffset + (screenWidth - xoffset) / 2;

    blocks = new Block[7] { IBlock(cellSize), JBlock(cellSize), LBlock(cellSize), OBlock(cellSize), SBlock(cellSize), TBlock(cellSize), ZBlock(cellSize)};
    currentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    gameOver = false;
    score = 0;
    highscore = LoadHighscoreFromFile("/tetris.txt");
    highscoreText.Format("%i", highscore);

    sndFlip = new Sound("/audio/tetris_rotate.raw", 2, 16);
    sndSuccess = new Sound("/audio/tetris_success.raw", 2, 16);
    writer = new FontWriter("/gfx/fonts/font_retropixel_20x20.lmi", {20,20});
    
    
    SetActive(true);
}

TetrisGame::~TetrisGame()
{
    delete[] blocks;
    delete sndFlip;
    delete sndSuccess;
    delete writer;

}

Block TetrisGame::GetRandomBlock()
{
    int randomIndex = rnd.GetNumber() % 7;
    Block block = blocks[randomIndex];    
    return block;
}


void TetrisGame::Draw(C2DGraphics *graphics)
{


    DrawUI(graphics);
    grid.Draw(graphics );
    currentBlock.Draw(SPACING, SPACING, graphics);
    switch (nextBlock.id)
    {
    case 3: //I
        nextBlock.Draw(centerOfFreeSpace- grid.GetWidth()/2, 240+cellSize*1.5, graphics);
        break;
    case 4: //O
        nextBlock.Draw(centerOfFreeSpace - grid.GetWidth()/2 + cellSize, 240+cellSize, graphics);
        break;
    default:
        nextBlock.Draw(centerOfFreeSpace - grid.GetWidth()/2 + cellSize/2, 240+cellSize, graphics);
        break;
    }


}


void TetrisGame::DrawUI(C2DGraphics *graphics)
{
    //Clear
    graphics->ClearScreen(BLACK_COLOR);

    Rectangle nextBox = {float(centerOfFreeSpace-cellSize*2), 240, float(cellSize*4), float(cellSize*4)};
    //Score
    scoreText.Format("%i", score);
    writer->Write(centerOfFreeSpace, 15, "Highscore", graphics, C2DGraphics::AlignCenter);
    writer->Write(centerOfFreeSpace, 45, highscoreText, graphics, C2DGraphics::AlignCenter);
    
    writer->Write(centerOfFreeSpace, 100, "Score", graphics, C2DGraphics::AlignCenter);
    writer->Write(centerOfFreeSpace, 130, scoreText, graphics, C2DGraphics::AlignCenter);

    //Next block    
    writer->Write(centerOfFreeSpace, nextBox.y-40, "Next", graphics, C2DGraphics::AlignCenter);    
    graphics->DrawRect(nextBox.x, nextBox.y, nextBox.width, nextBox.height, lightBlue);
    graphics->DrawRectOutline(nextBox.x, nextBox.y, nextBox.width, nextBox.height, WHITE_COLOR);
    
    

    //debugText.Format("xoffset: %i, cell: %ix%i, screen: %ix%i", xoffset, cellSize, cellSize, screenWidth, screenHeight);
    //graphics->DrawText(xoffset, screenHeight-50, WHITE_COLOR, debugText);
	
    if (gameOver) {
        writer->Write(centerOfFreeSpace, nextBox.y+nextBox.height+40, "GAME OVER", graphics, C2DGraphics::AlignCenter);    
    }

}



void TetrisGame::Update(CTimer *timer)
{
    if(EventTriggered(gameSpeed, timer->GetTicks()))
    {
        MoveBlockDown();
    }
}

void TetrisGame::HandleInput(TGamePadState gamePadState)
{
	if(keyDelay > 0)
	{
		keyDelay--;
		return;
	}  

    if (gameOver && ((gamePadState.buttons & GamePadButtonTriangle) || (gamePadState.buttons == SimpleGamePadButtonA)))
    {
        gameOver = false;
        Reset();
    }
    
    if ((gamePadState.buttons & GamePadButtonDown)){        
        MoveBlockDown();
        UpdateScore(0, 1);
    }
    if ((gamePadState.buttons & GamePadButtonUp) || (gamePadState.buttons & GamePadButtonCross /*also equals SimpleGamePadStart*/) 
 || (gamePadState.buttons & SimpleGamePadButtonB) ){        
        RotateBlock();
        keyDelay = 10;
    }
    if ((gamePadState.buttons & GamePadButtonLeft)){            
        MoveBlockLeft();
        keyDelay = 10;
    }
    if ((gamePadState.buttons & GamePadButtonRight)){            
        MoveBlockRight();
        keyDelay = 10;
    }
    if ((gamePadState.buttons & GamePadButtonStart) || (gamePadState.buttons & SimpleGamePadButtonSelect /*also equals GamePadCircle*/ )) {
        this->SetActive(false);
    }
    

}

void TetrisGame::MoveBlockLeft()
{
    if (!gameOver)
    {
        currentBlock.Move(0, -1);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(0, 1);
        }
    }
}

void TetrisGame::MoveBlockRight()
{
    if (!gameOver)
    {
        currentBlock.Move(0, 1);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(0, -1);
        }
    }
}

void TetrisGame::MoveBlockDown()
{
    if (!gameOver)
    {
        currentBlock.Move(1, 0);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(-1, 0);
            LockBlock();
        }
    }
}

bool TetrisGame::IsBlockOutside()
{
    Position *tiles = currentBlock.GetCellPositions();

    for (int i = 0; i < 4; i++)
    {
        Position item = (Position)tiles[i];

        if (grid.IsCellOutside(item.row, item.column))
        {
            return true;
        }
    }
    return false;
}

void TetrisGame::RotateBlock()
{
    if (!gameOver)
    {
        currentBlock.Rotate();
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.UndoRotation();
        }
        else
        {            
            sndFlip->Play(soundDevice);
        }
    }
}

void TetrisGame::LockBlock()
{
    Position *tiles = currentBlock.GetCellPositions();

    for (int i = 0; i < 4; i++)
    {
        Position item = (Position)tiles[i];
        grid.grid[item.row][item.column] = currentBlock.id;
    }
    currentBlock = nextBlock;
    if (BlockFits() == false)
    {
        gameOver = true;
    }
    nextBlock = GetRandomBlock();
    int rowsCleared = grid.ClearFullRows();
    if (rowsCleared > 0)
    {
        sndSuccess->Play(soundDevice);
        UpdateScore(rowsCleared, 0);
    }
}

bool TetrisGame::BlockFits()
{
    Position *tiles = currentBlock.GetCellPositions();

    for (int i = 0; i < 4; i++)
    {
        Position item = (Position)tiles[i];
        if (grid.IsCellEmpty(item.row, item.column) == false)
        {
            return false;
        }
    }
    return true;
}

void TetrisGame::Reset()
{
    grid.Initialize();
    currentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    score = 0;
}

void TetrisGame::UpdateScore(int linesCleared, int moveDownPoints)
{
    switch (linesCleared)
    {
    case 1:
        score += 100;
        break;
    case 2:
        score += 300;
        break;
    case 3:
        score += 500;
        break;
    default:
        break;
    }

    score += moveDownPoints;
    CheckForHighScore();
}

void TetrisGame::CheckForHighScore()
{
    if(score > highscore) {
        highscore = score;
        highscoreText.Format("%i", highscore);
        SaveHighscoreToFile(highscore, "/tetris.txt");
    }
}

