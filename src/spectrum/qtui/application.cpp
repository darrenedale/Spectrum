//
// Created by darren on 25/02/2021.
//

#include <cassert>
#include <QStatusBar>
#include "application.h"

using namespace Spectrum::QtUi;

Application * Application::m_instance = nullptr;

Application::Application(int & argc, char ** argv)
: QApplication(argc, argv),
  m_mainWindow()
{
    assert(!m_instance);
    m_instance = this;

    setApplicationDisplayName(QStringLiteral("Spectrum"));
    setApplicationName(QStringLiteral("Spectrum"));
    setOrganizationDomain(QStringLiteral("net.equituk"));
    setOrganizationName(QStringLiteral("Equit"));

    m_mainWindow.show();
}

void Application::showMessage(const QString & message, int timeout)
{
    mainWindow().statusBar()->showMessage(message, timeout);
}
