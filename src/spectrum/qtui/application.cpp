//
// Created by darren on 25/02/2021.
//

#include <cassert>
#include <QStatusBar>
#include <QIcon>
#include <QStringBuilder>
#include "application.h"
#include "notification.h"

using namespace Spectrum::QtUi;

Application * Application::m_instance = nullptr;

Application::Application(int & argc, char ** argv)
: QApplication(argc, argv),
  m_mainWindow(nullptr)
{
    assert(!m_instance);
    m_instance = this;

    setWindowIcon(icon(QStringLiteral("app")));
    setApplicationDisplayName(QStringLiteral("Spectrum"));
    setApplicationName(QStringLiteral("Spectrum"));
    setOrganizationDomain(QStringLiteral("net.equituk"));
    setOrganizationName(QStringLiteral("Equit"));

    // we must initialise this after the application name etc. have been set because the main window constructor depends on some of these properties
    m_mainWindow = std::make_unique<MainWindow>();
    m_mainWindow->show();
}

void Application::showMessage(const QString & message, int timeout)
{
    Notification::showNotification(message, timeout);
//    mainWindow().statusBar()->showMessage(message, timeout);
}

Application::ThemeType Application::themeType() const
{
    if (128 <= palette().window().color().lightness()) {
        return ThemeType::Light;
    }

    return ThemeType::Dark;
}

QIcon Application::icon(const QString & name) const
{
    switch (themeType()) {
        case ThemeType::Dark:
            return QIcon(QStringLiteral(":/icons/dark/") % name);

        case ThemeType::Light:
        default:
            return QIcon(QStringLiteral(":/icons/light/") % name);
    }
}
