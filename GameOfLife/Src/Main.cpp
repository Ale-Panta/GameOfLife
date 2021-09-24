#include "Game/GameOfLife.h"

using namespace std;

int main()
{
	GameOfLife app(100, 60, 2);
	app.Init();

	return app.Run();
}
