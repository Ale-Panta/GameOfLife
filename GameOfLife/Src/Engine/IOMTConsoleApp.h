#pragma once
#include <windows.h>
#include <iostream>
#include <atomic>
#include <thread>

class IOMTConsoleApp
{
public:
	IOMTConsoleApp(SHORT screenWith, SHORT screenHeight, const SHORT numOfSBO); // #TODO parametrize it
	virtual ~IOMTConsoleApp();

	// Deleting default constructor, copy constructor, assign operator
	IOMTConsoleApp() = delete;
	IOMTConsoleApp(const IOMTConsoleApp& rhs) = delete;
	IOMTConsoleApp operator = (const IOMTConsoleApp& rhs) = delete;

	/**
	 * Initialize the console engine.
	 * Create the COUT consoles. The CIN thread is created in another thread
	 * and its event loop is started here.
	 */
	void Init();

	/**
	 * Run the application executing the Update and Render.
	 * Render is treated as Present and Swap action (roughly). 
	 * 
	 * \return <code>true</code> if the application end successfully
	 */
	bool Run();

protected:
	/**
	 * Execute the key codes events.
	 */
	virtual void KeyEventProc(KEY_EVENT_RECORD keyEvent);

	/**
	 * Execute the mouse events.
	 */
	virtual void MouseEventProc(MOUSE_EVENT_RECORD mouseEvent);

	/**
	 * Update the game logic.
	 * There is no time reference, the engine slow down the process itself.
	 */
	virtual void Update();

	/**
	 * Draw the element in the screen buffer.
	 * 
	 * \param x	columns
	 * \param y	row
	 * \param character	UniCode char
	 * \param color
	 */
	void Draw(SHORT x, SHORT y, SHORT character = 0x2588, SHORT color = 0x000F);

	bool mIsUpdateEnabled;
	bool mIsApplicationRunning;

	COORD mScreenSize;
	SMALL_RECT mClientRect;
	const SHORT mScreenWidth;
	const SHORT mScreenHeight;
	CHAR_INFO* mpScreenBuffer;

private:
	void ReadCin();
	void CreateCout();
	void CreateCin();
	void ErrorExit(LPSTR Message);

	SHORT mNumOfSBO;
	const SHORT mMinSBO = 1;
	const SHORT mMaxSBO = 3;

	HANDLE* mhOuts;
	HANDLE mhIn;

	DWORD mInOldMode;

	static const SHORT mNumOfEventsToRead = 16;
	INPUT_RECORD mIrInBuf[mNumOfEventsToRead];

	std::thread mCinThread;
};

