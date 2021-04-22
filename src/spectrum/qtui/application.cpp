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
  m_mainWindow(nullptr)
{
    assert(!m_instance);
    m_instance = this;

    setApplicationDisplayName(QStringLiteral("Spectrum"));
    setApplicationName(QStringLiteral("Spectrum"));
    setOrganizationDomain(QStringLiteral("net.equituk"));
    setOrganizationName(QStringLiteral("Equit"));
    setProperty("version", QStringLiteral("0.5"));
    setProperty("author", QStringLiteral("Darren Edale"));

    // we must initialise this after the application name etc. have been set because the main window constructor depends on some of these properties
    m_mainWindow = std::make_unique<MainWindow>();
    m_mainWindow->show();
}

void Application::showMessage(const QString & message, int timeout)
{
    mainWindow().statusBar()->showMessage(message, timeout);
}
