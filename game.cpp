#include "game.h"
#include <random>
#include <string>

Game::Game(){
    grid = Grid();
    blocks = GetAllBlocks();
    currentblock = GetRandomBlock();
    nextblock = GetRandomBlock();
    gameOver = false;
    score = 0;
    level = 1;
    totalLines = 0;
}

Block Game::GetRandomBlock(){
    if(blocks.empty()) blocks = GetAllBlocks();
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, blocks.size() - 1);

    int randomIndex = dist(gen);
    Block block = blocks[randomIndex];
    blocks.erase(blocks.begin() + randomIndex);
    return block;
}

vector <Block> Game::GetAllBlocks(){
    return {Iblock(), Oblock(), Tblock(), Jblock(), Lblock(), Sblock(), Zblock()};
}

void Game::Draw(){
    grid.Draw();
    currentblock.Draw();
    DrawStats();
    if(gameOver) DrawGameOverScreen();
}

void Game::DrawStats(){
    const int statsX = 330;
    DrawRectangle(300, 0, 200, 600, Color{30, 30, 90, 255});
    DrawText("Score", statsX, 100, 24, WHITE);
    DrawText(to_string(score).c_str(), statsX, 130, 24, WHITE);
    DrawText("Level", statsX, 200, 24, WHITE);
    DrawText(to_string(level).c_str(), statsX, 230, 24, WHITE);
    DrawText("Lines Cleared", statsX, 300, 24, WHITE);
    DrawText(to_string(totalLines).c_str(), statsX, 330, 24, WHITE);
}

void Game::DrawGameOverScreen(){
    const int panelX = 45;
    const int panelY = 60;
    const int panelWidth = 410;
    const int panelHeight = 480;
    const int panelCenter = panelX + panelWidth / 2;
    const Color panelColor = {25, 25, 65, 255};
    const Color accentColor = {120, 220, 255, 255};

    DrawRectangle(0, 0, 500, 600, Color{0, 0, 0, 175});
    DrawRectangleRounded({panelX + 5.0f, panelY + 5.0f, panelWidth, panelHeight}, 0.08f, 12, Color{0, 0, 0, 100});
    DrawRectangleRounded({panelX, panelY, panelWidth, panelHeight}, 0.08f, 12, panelColor);
    DrawRectangleRoundedLines({panelX, panelY, panelWidth, panelHeight}, 0.08f, 12, accentColor);

    const char* title = "GAME OVER";
    DrawText(title, panelCenter - MeasureText(title, 42) / 2, 95, 42, accentColor);

    const char* finalScoreLabel = "Final Score";
    const char* levelLabel = "Level";
    const char* linesLabel = "Lines Cleared";
    const string scoreText = to_string(score);
    const string levelText = to_string(level);
    const string linesText = to_string(totalLines);

    DrawText(finalScoreLabel, panelCenter - MeasureText(finalScoreLabel, 22) / 2, 180, 22, LIGHTGRAY);
    DrawText(scoreText.c_str(), panelCenter - MeasureText(scoreText.c_str(), 30) / 2, 208, 30, WHITE);
    DrawText(levelLabel, panelCenter - MeasureText(levelLabel, 22) / 2, 270, 22, LIGHTGRAY);
    DrawText(levelText.c_str(), panelCenter - MeasureText(levelText.c_str(), 30) / 2, 298, 30, WHITE);
    DrawText(linesLabel, panelCenter - MeasureText(linesLabel, 22) / 2, 360, 22, LIGHTGRAY);
    DrawText(linesText.c_str(), panelCenter - MeasureText(linesText.c_str(), 30) / 2, 388, 30, WHITE);

    const char* restartMessage = "Press any key to restart";
    DrawText(restartMessage, panelCenter - MeasureText(restartMessage, 20) / 2, 475, 20, accentColor);
}

void Game::HandleInput(){
    int keypressed = GetKeyPressed();

    if(gameOver && keypressed != 0){
        gameOver = false;
        Reset();
        return;
    }

    switch(keypressed){
        case KEY_LEFT:
        MoveLeft();
        break;

        case KEY_RIGHT:
        MoveRight();
        break;

        case KEY_DOWN:
        MoveDown();
        break;

        case KEY_UP:
        RotateBlock();
        break;

        default:
        break;
    }
}

void Game::MoveLeft(){
    if(!gameOver){
        currentblock.Move(0,-1);
        if(IsBlockOutside() || BlockFits() == false) currentblock.Move(0,1);
    }
}

void Game::MoveRight(){
    if(!gameOver){
        currentblock.Move(0,1);
        if(IsBlockOutside() || BlockFits() == false) currentblock.Move(0,-1);
    }
}

void Game::MoveDown(){
    if(!gameOver){
        currentblock.Move(1,0);
        if(IsBlockOutside() || BlockFits() == false) {
            currentblock.Move(-1,0);
            LockBlock();
        }
    }
}

void Game::MoveUp(){
    if(!gameOver) RotateBlock();
}

void Game::LockBlock(){
    for(const auto item: currentblock.GetCellPosition()){
        grid.grid[item.row][item.column] = currentblock.id;
    }

    currentblock = nextblock;
    if(BlockFits() == false){
        gameOver = true;
    }
    nextblock = GetRandomBlock();

    int clearedLines = grid.ClearFullRows();
    UpdateScore(clearedLines);
    UpdateLevel();
}

void Game::UpdateScore(int clearedLines){
    switch(clearedLines){
        case 1:
            score += 100 * level;
            break;
        case 2:
            score += 300 * level;
            break;
        case 3:
            score += 500 * level;
            break;
        case 4:
            score += 800 * level;
            break;
        default:
            break;
    }

    totalLines += clearedLines;
}

void Game::UpdateLevel(){
    level = totalLines / 10 + 1;
}

bool Game::IsBlockOutside()
{
    for(const auto item: currentblock.GetCellPosition()){
        if(grid.IsCellOutside(item.row, item.column)){
            return true;
        }
    }
    return false;
}

void Game::RotateBlock(){
    currentblock.Rotate();

    if(IsBlockOutside()){
        currentblock.UndoRotate();
    }
}

bool Game::BlockFits(){
    for(const auto item: currentblock.GetCellPosition()){
        if(grid.IsCellEmpty(item.row,item.column) == false){
            return false;
        }
    }
    return true;
}

void Game::Reset(){
    grid.Initialize();
    blocks = GetAllBlocks();
    currentblock = GetRandomBlock();
    nextblock = GetRandomBlock();
    score = 0;
    level = 1;
    totalLines = 0;
}