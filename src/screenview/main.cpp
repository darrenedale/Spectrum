#include "application.h"
#include "mainwindow.h"

int main( int argc, char * argv[] )
{
	ScreenView::Application app(argc, argv);
    QApplication::quitOnLastWindowClosed();
    ScreenView::MainWindow win;
	win.show();
	return QApplication::exec();
}
