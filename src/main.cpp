
#include <cstdio>
#include <iostream>
#include "app.hpp"

int main()
{
	App app;
	app.Initialize("Dandy VR Remap", 900, 800);
	app.Run();
	return 0;
}