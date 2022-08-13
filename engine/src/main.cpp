#include <application.h>

int main(int argc, char* argv[])
{
	Sunset::Application app;

	app.init();
	
	app.run();

	app.cleanup();

	return 0;
}
