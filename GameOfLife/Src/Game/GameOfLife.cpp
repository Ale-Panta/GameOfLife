#include "GameOfLife.h"

GameOfLife::GameOfLife(SHORT screenWith, SHORT screenHeight, const SHORT numOfSBO) 
	: IOMTConsoleApp::IOMTConsoleApp(screenWith, screenHeight, numOfSBO)
{
    mOutput = new SHORT[mScreenWidth * mScreenHeight];
    memset(mOutput, 0, sizeof(SHORT) * mScreenWidth * mScreenHeight);

    mState = new SHORT[mScreenWidth * mScreenHeight];
    memset(mState, 0, sizeof(SHORT) * mScreenWidth * mScreenHeight);
}

GameOfLife::~GameOfLife()
{
    delete[] mState;
    delete[] mOutput;

    printf("Thanks to play GameOfLife \nMade by Alessandro Pantano \n");
}

void GameOfLife::KeyEventProc(KEY_EVENT_RECORD keyEvent)
{
    if (keyEvent.bKeyDown)
    {

    }
    else if (!keyEvent.bKeyDown)
    {
        if (keyEvent.uChar.AsciiChar == 'p')
        {
            mIsUpdateEnabled = !mIsUpdateEnabled;
        }
        else if (keyEvent.uChar.AsciiChar == 'c')
        {
            memset(mOutput, 0, sizeof(SHORT) * mScreenWidth * mScreenHeight);
            memset(mState, 0, sizeof(SHORT) * mScreenWidth * mScreenHeight);
            memset(mpScreenBuffer, 0, sizeof(CHAR_INFO) * mScreenWidth * mScreenHeight);
        }
        else if (keyEvent.uChar.AsciiChar == 'e')
        {
            mIsApplicationRunning = false;
        }
    }
}

void GameOfLife::MouseEventProc(MOUSE_EVENT_RECORD mouseEvent)
{
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif

    switch (mouseEvent.dwEventFlags)
    {
    case 0:

        if (mouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            Draw(mouseEvent.dwMousePosition.X, mouseEvent.dwMousePosition.Y);
            mState[mouseEvent.dwMousePosition.Y * mScreenWidth + mouseEvent.dwMousePosition.X] = 1;
        }
        else if (mouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
        {
        }
        else
        {
        }
        break;

    case DOUBLE_CLICK:
        break;

    case MOUSE_HWHEELED:
        break;

    case MOUSE_MOVED:
        if (mouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            Draw(mouseEvent.dwMousePosition.X, mouseEvent.dwMousePosition.Y);
            mState[mouseEvent.dwMousePosition.Y * mScreenWidth + mouseEvent.dwMousePosition.X] = 1;
        }
        break;

    default:
        break;
    }
}

void GameOfLife::Update()
{
    if (!mIsUpdateEnabled)
        return;

    // Temporary Update
    auto cell = [ & ](SHORT x, SHORT y)
    {
        if (x < 0) x = mScreenWidth - 1;
        else if (x > mScreenWidth - 1) x = 0;
        if (y < 0) y = mScreenHeight - 1;
        else if (y > mScreenHeight - 1) y = 0;

        return mOutput[y * mScreenWidth + x];
    };

    // Store output state
    for (SHORT i = 0; i < mScreenWidth * mScreenHeight; i++)
    {
        mOutput[i] = mState[i];
    }

    for (SHORT x = 0; x < mScreenWidth; x++)
    {
        for (SHORT y = 0; y < mScreenHeight; y++)
        {
            SHORT neighbours =
                cell(x - 1, y - 1) + cell(x, y - 1) + cell(x + 1, y - 1) +
                cell(x - 1, y) + 0 + cell(x + 1, y + 0) +
                cell(x - 1, y + 1) + cell(x, y + 1) + cell(x + 1, y + 1);

            if (cell(x, y) == 1)
                mState[y * mScreenWidth + x] = neighbours == 2 || neighbours == 3;
            else
                mState[y * mScreenWidth + x] = neighbours == 3;

            if (cell(x, y) == 1)
                Draw(x, y, 9608, 0x000F);
            else
                Draw(x, y, 9608, 0x0000);
        }
    }
}
