#pragma once
#include "../Engine/IOMTConsoleApp.h"

class GameOfLife : public IOMTConsoleApp
{
public:
	GameOfLife(SHORT screenWith, SHORT screenHeight, const SHORT numOfSBO);
	virtual ~GameOfLife();

protected:
	virtual void KeyEventProc(KEY_EVENT_RECORD keyEvent);
	virtual void MouseEventProc(MOUSE_EVENT_RECORD mouseEvent);

	virtual void Update();

private:
	SHORT *mOutput;
	SHORT *mState;
};