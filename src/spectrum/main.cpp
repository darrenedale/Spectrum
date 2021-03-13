#include "qt/application.h"
#include "qt/mainwindow.h"

int main(int argc, char * argv[])
{
    using namespace Spectrum::Qt;

	Application app(argc, argv);
	MainWindow win;
	win.show();
	return Application::exec();
}
