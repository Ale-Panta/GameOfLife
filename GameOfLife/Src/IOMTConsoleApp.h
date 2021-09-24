#pragma once
#include <windows.h>
#include <iostream>
#include <atomic>
#include <thread>

class IOMTConsoleApp
{
public:
	IOMTConsoleApp(SHORT screenWith, SHORT screenHeight); // #TODO parametrize it
	virtual ~IOMTConsoleApp();

	void Init();

protected:
	virtual void ReadCin();
	virtual void KeyEventProc(KEY_EVENT_RECORD keyEvent);
	virtual void MouseEventProc(MOUSE_EVENT_RECORD mouseEvent);

	virtual void Update();
	virtual void Draw(SHORT x, SHORT y, SHORT character = 0x2588, SHORT color = 0x000F);

private:
	void CreateCout();
	void CreateCin();
	void ErrorExit(LPSTR Message);

	bool mIsUpdateEnabled;
	bool mIsApplicationRunning;

	HANDLE mhOuts[2];
	HANDLE mhIn;

	COORD mScreenSize;
	SMALL_RECT mClientRect;
	SHORT mScreenWidth;
	SHORT mScreenHeight;
	DWORD mInOldMode;
	CHAR_INFO* mpScreenBuffer;

	static const SHORT mNumOfEventsToRead = 16;
	INPUT_RECORD mIrInBuf[mNumOfEventsToRead];

	std::thread mCinThread;

	// Game of life
	SHORT *mOutput;
	SHORT *mState;
};

