#include "app.h"

int main(int argc, char** argv) {
	static_cast<void>(argc);
	static_cast<void>(argv);

	Application app;
	app.run();

	system("pause");

	return 0;
}