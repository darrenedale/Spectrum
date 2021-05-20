//
// Created by darren on 25/02/2021.
//

#include "application.h"

using namespace Spectrum::ScreenView;

Application::Application(int & argc, char ** argv)
: QApplication(argc, argv)
{
    setApplicationDisplayName(QStringLiteral("Spectrum Screen Viewer"));
    setApplicationName(QStringLiteral("SpectrumScreenView"));
    setOrganizationDomain(QStringLiteral("net.equituk"));
    setOrganizationName(QStringLiteral("Equit"));
}
