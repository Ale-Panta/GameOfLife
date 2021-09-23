#include "IOMTConsoleApp.h"

#include <ctime>

using namespace std;

IOMTConsoleApp::IOMTConsoleApp(SHORT screenWith, SHORT screenHeight)
    : mScreenWidth(screenWith)
    , mScreenHeight(screenHeight)
    , mInOldMode(0)
    , mhOut(0)
    , mhIn(0)
    , mpScreenBuffer(nullptr)
    , mIsUpdateEnabled(false)
    , mIsApplicationRunning(true)
{
    mScreenSize = { mScreenWidth, mScreenHeight };
    mClientRect = { 0, 0, (SHORT)(mScreenWidth - 1), (SHORT)(mScreenHeight - 1) };

    mOutput = new SHORT[mScreenWidth * mScreenHeight];
    memset(mOutput, 0, sizeof(SHORT) * mScreenWidth * mScreenHeight);

    mState = new SHORT[mScreenWidth * mScreenHeight];
    memset(mState, 0, sizeof(SHORT) * mScreenWidth * mScreenHeight);

    mpScreenBuffer = new CHAR_INFO[mScreenWidth * mScreenHeight];
    memset(mpScreenBuffer, 0, sizeof(CHAR_INFO) * mScreenWidth * mScreenHeight);
}

IOMTConsoleApp::~IOMTConsoleApp()
{
    delete[] mpScreenBuffer;
    delete[] mState;
    delete[] mOutput;
}

void IOMTConsoleApp::Init()
{
    thread cinThread(&IOMTConsoleApp::ReadCin, this);
    CreateCout();

    double previous = clock();
    double lag = 0.0;
    
    while (mIsApplicationRunning)
    {
    	double current = clock();
    	double elapsed = current - previous;
    	previous = current;
    	lag += elapsed;
    		
    	while (lag >= 100.0)
    	{
            Update();
    		lag -= 100.0;
    	}
    
        // Rendering
        WriteConsoleOutput(mhOut, mpScreenBuffer, mScreenSize, { 0, 0 }, &mClientRect);
    }

    cinThread.join();

}

void IOMTConsoleApp::ReadCin()
{
    CreateCin();

    while (mIsApplicationRunning)
    {
        // Event loop
        DWORD numberEventsRead;

        if (!ReadConsoleInput(mhIn, mIrInBuf, 16, &numberEventsRead))
            ErrorExit(LPSTR("ReadConsoleInput"));

        for (DWORD i = 0; i < numberEventsRead; i++)
        {
            switch (mIrInBuf[i].EventType)
            {
            case KEY_EVENT:
                KeyEventProc(mIrInBuf[i].Event.KeyEvent);
                break;

            case MOUSE_EVENT:
                MouseEventProc(mIrInBuf[i].Event.MouseEvent);
                break;

            case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                break;

            case FOCUS_EVENT:  // disregard focus events
                break;

            case MENU_EVENT:   // disregard menu events
                break;

            default:
                break;
            }
        }
    }

    SetConsoleMode(mhIn, mInOldMode);
}

void IOMTConsoleApp::KeyEventProc(KEY_EVENT_RECORD keyEvent)
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

void IOMTConsoleApp::MouseEventProc(MOUSE_EVENT_RECORD mouseEvent)
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

void IOMTConsoleApp::Update()
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

void IOMTConsoleApp::Draw(SHORT x, SHORT y, SHORT character, SHORT color)
{
    if (x <= mScreenWidth && x >= 0 && y <= mScreenHeight && y >= 0)
    {
        mpScreenBuffer[y * mScreenWidth + x].Char.UnicodeChar = character;
        mpScreenBuffer[y * mScreenWidth + x].Attributes = color;
    }
}

void IOMTConsoleApp::CreateCout()
{
	CONSOLE_CURSOR_INFO cursorInfo { 1, FALSE };
	CONSOLE_FONT_INFOEX consoleInfo { sizeof(consoleInfo), 0, 6, 6, FF_DONTCARE, FW_NORMAL };

    mhOut = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    // Mode and screen buffer settings
    if (!SetConsoleMode(mhOut, ENABLE_PROCESSED_OUTPUT))
        ErrorExit(LPSTR("SetConsoleMode"));

    if (!SetConsoleActiveScreenBuffer(mhOut))
        ErrorExit(LPSTR("SetConsoleActiveScreenBuffer"));

    // In order to set the screen buffer we need to shrink the window to the minimum size.
    SMALL_RECT tempWindowRect = { 0, 0, 1, 1 };
    if (!SetConsoleWindowInfo(mhOut, TRUE, &tempWindowRect))
        ErrorExit(LPSTR("SetConsoleWindowInfo"));

    if (!SetConsoleScreenBufferSize(mhOut, mScreenSize))
        ErrorExit(LPSTR("SetConsoleScreenBufferSize"));

    // Font settings and window size checks
    if (!SetCurrentConsoleFontEx(mhOut, FALSE, &consoleInfo))
        ErrorExit(LPSTR("SetCurrentConsoleFontEx"));

    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
    if (!GetConsoleScreenBufferInfo(mhOut, &consoleScreenBufferInfo))
        ErrorExit(LPSTR("GetConsoleScreenBufferInfo"));

    if (mScreenWidth > consoleScreenBufferInfo.dwMaximumWindowSize.X)
        ErrorExit(LPSTR("Screen width or font width too big"));

    if (mScreenHeight > consoleScreenBufferInfo.dwMaximumWindowSize.Y)
        ErrorExit(LPSTR("Screen height or font height too big"));

    // Resetting console size
    if (!SetConsoleWindowInfo(mhOut, TRUE, &mClientRect))
        ErrorExit(LPSTR("SetConsoleWindowInfo1"));

    // Cursor settings
    if (!SetConsoleCursorInfo(mhOut, &cursorInfo))
        ErrorExit(LPSTR("SetConsoleCursorInfo"));
}

void IOMTConsoleApp::CreateCin()
{
    mhIn = GetStdHandle(STD_INPUT_HANDLE);

    if (mhIn == INVALID_HANDLE_VALUE)
        ErrorExit(LPSTR("Invalid handle value"));

    if (!GetConsoleMode(mhIn, &mInOldMode))
        ErrorExit(LPSTR("GetConsoleMode"));

    if (!SetConsoleMode(mhIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS))
        ErrorExit(LPSTR("SetConsoleMode"));
}

void IOMTConsoleApp::ErrorExit(LPSTR Message)
{
    fprintf(stderr, "%s\n", Message);

    // Restore input mode on exit
    SetConsoleMode(mhIn, mInOldMode);

    ExitProcess(0);
}