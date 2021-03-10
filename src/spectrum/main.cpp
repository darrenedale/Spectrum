#include "qt/application.h"
#include "qt/mainwindow.h"

int main( int argc, char * argv[] )
{
	Spectrum::Qt::Application app(argc, argv);
	Spectrum::Qt::MainWindow win;
	win.show();
	return QApplication::exec();
}
