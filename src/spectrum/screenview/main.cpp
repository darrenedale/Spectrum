#include "application.h"
#include "mainwindow.h"

int main( int argc, char * argv[] )
{
    using namespace Spectrum::ScreenView;

	Application app(argc, argv);
    Application::quitOnLastWindowClosed();
    MainWindow win;
	win.show();
	return Application::exec();
}
