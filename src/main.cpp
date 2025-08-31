
#include <cstdio>
#include <iostream>
#include "app.hpp"

#include <cmgCore/cmg_core.h>
#include <cmgApplication/cmg_application.h>

int main()
{
	App app;
	app.Initialize("Dandy VR Remap", 900, 600);
	app.Run();
	return 0;
}