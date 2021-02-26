//
// Created by darren on 25/02/2021.
//

#include "application.h"

using namespace Spectrum;

Application::Application(int & argc, char ** argv)
: QApplication(argc, argv)
{
    setApplicationDisplayName(QStringLiteral("Spectrum"));
    setApplicationName(QStringLiteral("Spectrum"));
    setOrganizationDomain(QStringLiteral("net.equituk"));
    setOrganizationName(QStringLiteral("Equit"));
}
