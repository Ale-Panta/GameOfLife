#include "IOMTConsoleApp.h"

#include <ctime>

using namespace std;

IOMTConsoleApp::IOMTConsoleApp(SHORT screenWith, SHORT screenHeight, const SHORT numOfSBO)
    : mScreenWidth(screenWith)
    , mScreenHeight(screenHeight)
    , mInOldMode(0)
    , mhIn(0)
    , mNumOfSBO(numOfSBO)
    , mhOuts(nullptr)
    , mpScreenBuffer(nullptr)
    , mIsUpdateEnabled(false)
    , mIsApplicationRunning(true)
{
    // Clamp multiple buffers value
    if (mNumOfSBO < mMinSBO) 
        mNumOfSBO = mMinSBO;
    else if (mNumOfSBO > mMaxSBO) 
        mNumOfSBO = mMaxSBO;

    // Allocate memory for screen buffer consoles
    mhOuts = new HANDLE[mNumOfSBO];

    // Define screen size and rect zone
    mScreenSize = { mScreenWidth, mScreenHeight };
    mClientRect = { 0, 0, (SHORT)(mScreenWidth - 1), (SHORT)(mScreenHeight - 1) };

    // Allocate memory for screen buffer
    mpScreenBuffer = new CHAR_INFO[mScreenWidth * mScreenHeight];
    memset(mpScreenBuffer, 0, sizeof(CHAR_INFO) * mScreenWidth * mScreenHeight);
}

IOMTConsoleApp::~IOMTConsoleApp()
{
    delete[] mpScreenBuffer;

    if (mCinThread.joinable())
        mCinThread.join();

    delete[] mhOuts;
}

void IOMTConsoleApp::Init()
{
    mCinThread = thread(&IOMTConsoleApp::ReadCin, this);
    CreateCout();
}

bool IOMTConsoleApp::Run()
{
    double previous = clock();
    double lag = 0.0;
    SHORT i = 0;

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

        WriteConsoleOutput(mhOuts[i], mpScreenBuffer, mScreenSize, { 0, 0 }, &mClientRect);
        if (!SetConsoleActiveScreenBuffer(mhOuts[i]))
            return 1;

        i = (i + 1) % mNumOfSBO;
    }

    mCinThread.join();

    return 0;
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

            default:
                break;
            }
        }
    }

    SetConsoleMode(mhIn, mInOldMode);
}

void IOMTConsoleApp::KeyEventProc(KEY_EVENT_RECORD keyEvent)
{
    //if (keyEvent.bKeyDown)
    //{

    //}
    //else if (!keyEvent.bKeyDown)
    //{
    //    if (keyEvent.uChar.AsciiChar == 'p')
    //    {
    //    }
    //}
}

void IOMTConsoleApp::MouseEventProc(MOUSE_EVENT_RECORD mouseEvent)
{
//#ifndef MOUSE_HWHEELED
//#define MOUSE_HWHEELED 0x0008
//#endif
//
//    switch (mouseEvent.dwEventFlags)
//    {
//    case 0:
//
//        if (mouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
//        {
//        }
//        else if (mouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
//        {
//        }
//        else
//        {
//        }
//        break;
//
//    case DOUBLE_CLICK:
//        break;
//
//    case MOUSE_HWHEELED:
//        break;
//
//    case MOUSE_MOVED:
//        if (mouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
//        {
//        }
//        break;
//
//    default:
//        break;
//    }
}

void IOMTConsoleApp::Update()
{
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
    const CONSOLE_CURSOR_INFO cursorInfo { 1, FALSE };
    CONSOLE_FONT_INFOEX consoleInfo { sizeof(consoleInfo), 0, 8, 8, FF_DONTCARE, FW_NORMAL };

    for (SHORT i = 0; i < mNumOfSBO; i++)
    {
        mhOuts[i] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

        if (mhOuts[i] == INVALID_HANDLE_VALUE)
            ErrorExit(LPSTR("Invalid handle"));

        // In order to set the screen buffer we need to shrink the window to the minimum size.
        const SMALL_RECT tempWindowRect = { 0, 0, 1, 1 };
        if (!SetConsoleWindowInfo(mhOuts[i], TRUE, &tempWindowRect))
            ErrorExit(LPSTR("SetConsoleWindowInfo"));

        if (!SetConsoleScreenBufferSize(mhOuts[i], mScreenSize))
            ErrorExit(LPSTR("SetConsoleScreenBufferSize"));

        // Font settings and window size checks
        if (!SetCurrentConsoleFontEx(mhOuts[i], FALSE, &consoleInfo))
            ErrorExit(LPSTR("SetCurrentConsoleFontEx"));

        CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
        if (!GetConsoleScreenBufferInfo(mhOuts[i], &consoleScreenBufferInfo))
            ErrorExit(LPSTR("GetConsoleScreenBufferInfo"));

        if (mScreenWidth > consoleScreenBufferInfo.dwMaximumWindowSize.X)
            ErrorExit(LPSTR("Screen width or font width too big"));

        if (mScreenHeight > consoleScreenBufferInfo.dwMaximumWindowSize.Y)
            ErrorExit(LPSTR("Screen height or font height too big"));

        // Resetting console size
        if (!SetConsoleWindowInfo(mhOuts[i], TRUE, &mClientRect))
            ErrorExit(LPSTR("SetConsoleWindowInfo1"));

        // Cursor settings
        if (!SetConsoleCursorInfo(mhOuts[i], &cursorInfo))
            ErrorExit(LPSTR("SetConsoleCursorInfo"));
    }
}

void IOMTConsoleApp::CreateCin()
{
    mhIn = GetStdHandle(STD_INPUT_HANDLE);

    if (mhIn == INVALID_HANDLE_VALUE)
        ErrorExit(LPSTR("Invalid handle value"));

    if (!GetConsoleMode(mhIn, &mInOldMode))
        ErrorExit(LPSTR("GetConsoleMode"));

    if (!SetConsoleMode(mhIn, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS))
        ErrorExit(LPSTR("SetConsoleMode"));
}

void IOMTConsoleApp::ErrorExit(LPSTR Message)
{
    fprintf(stderr, "%s\n", Message);

    // Restore input mode on exit
    SetConsoleMode(mhIn, mInOldMode);

    ExitProcess(0);
}