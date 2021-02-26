#include "application.h"
#include "mainwindow.h"

int main( int argc, char * argv[] )
{
	Spectrum::Application app(argc, argv);
	Spectrum::MainWindow win;
	win.show();
	return QApplication::exec();
}
