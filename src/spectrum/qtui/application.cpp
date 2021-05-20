//
// Created by darren on 25/02/2021.
//

#include <cassert>
#include <QStatusBar>
#include <QIcon>
#include <QStringBuilder>
#include <QtGlobal>
#include <QStandardPaths>
#include <QFileInfo>
#include "mainwindow.h"
#include "application.h"
#include "notification.h"

#if (defined(Q_OS_MAC))
#include <CoreFoundation/CoreFoundation.h>
#endif

using namespace Spectrum::QtUi;

Application * Application::m_instance = nullptr;

Application::Application(int & argc, char ** argv)
: QApplication(argc, argv),
  m_mainWindow(nullptr)
{
    assert(!m_instance);
    m_instance = this;
    setWindowIcon(icon(QStringLiteral("app")));

    #if (defined(NDEBUG))
    setApplicationDisplayName(QStringLiteral("Spectrum"));
#else
    setApplicationDisplayName(QStringLiteral("Spectrum [Debug Build]"));
#endif

    setApplicationName(QStringLiteral("Spectrum"));
    setOrganizationDomain(QStringLiteral("net.equituk"));
    setOrganizationName(QStringLiteral("Equit"));
    setProperty("version", QStringLiteral("0.5"));
    setProperty("author", QStringLiteral("Darren Edale"));

    // we must initialise this after the application name etc. have been set because the main window constructor depends on some of these properties
    m_mainWindow = std::make_unique<MainWindow>();
    m_mainWindow->show();
}

void Application::showNotification(const QString & message, int timeout)
{
    Notification::showNotification(message, timeout);
}

Application::ThemeType Application::themeType()
{
    if (128 <= palette().window().color().lightness()) {
        return ThemeType::Light;
    }

    return ThemeType::Dark;
}

QIcon Application::icon(const QString & name, ThemeType type)
{
    if (ThemeType::Unknown == type) {
        type = themeType();
    }

    switch (type) {
        case ThemeType::Dark:
            return QIcon(QStringLiteral(":/icons/dark/") % name);

        case ThemeType::Light:
        default:
            return QIcon(QStringLiteral(":/icons/light/") % name);
    }
}

QIcon Application::icon(const QString & systemThemeName, const QString & name, Application::ThemeType type)
{
    return QIcon::fromTheme(systemThemeName, icon(name, type));
}

MainWindow & Application::mainWindow()
{
    return *m_mainWindow;
}

const MainWindow & Application::mainWindow() const
{
    return *m_mainWindow;
}

QString Application::romFilePath(const char * const romFile)
{
#if (defined(Q_OS_MAC))
    static QString basePath;

    if (basePath.isNull()) {
        basePath = QUrl::fromCFURL(static_cast<CFURLRef>(
                CFAutorelease(static_cast<CFURLRef>(CFBundleCopyBundleURL( CFBundleGetMainBundle())))
        )).path() % "/Contents/roms/";
    }

    auto path = basePath % romFile;

    if (QFileInfo::exists(path)) {
        return path;
    }

    return {};
#elif (defined(Q_OS_WIN) || defined(Q_OS_LINUX))
    return QStandardPaths::locate(QStandardPaths::StandardLocation::AppDataLocation, QStringLiteral("roms/") % romFile);
#endif
    return {};
}

Application::~Application() = default;
