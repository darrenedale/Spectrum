#include "qtui/application.h"

int main(int argc, char * argv[])
{
    using namespace Spectrum::QtUi;

	Application app(argc, argv);
	return Application::exec();
}
