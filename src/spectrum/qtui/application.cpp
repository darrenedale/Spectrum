//
// Created by darren on 25/02/2021.
//

#include <cassert>
#include <QStatusBar>
#include <QIcon>
#include <QStringBuilder>
#include <QFileInfo>
#include <QSettings>
#include "mainwindow.h"
#include "application.h"
#include "notification.h"

using namespace Spectrum::QtUi;

namespace
{
    /**
     * The maximum size of the recent snapshots list.
     */
    constexpr const int maxRecentSnapshots = 5;
}

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

    loadRecentSnapshots();

    // we must initialise this after the application name etc. have been set because the main window constructor depends on some of these properties
    m_mainWindow = std::make_unique<MainWindow>();
    m_mainWindow->show();
}

Application::~Application()
{
    saveRecentSnapshots();
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

void Application::addRecentSnapshot(QString fileName)
{
    // we store and compare based on absolute paths to avoid duplicates
    fileName = QFileInfo(fileName).absoluteFilePath();

    auto pos = std::find_if(m_recentSnapshots.begin(), m_recentSnapshots.end(), [&fileName] (const auto & recentSnapshot) -> bool {
        return QFileInfo(recentSnapshot).absoluteFilePath() == fileName;
    });

    if (pos != m_recentSnapshots.cend()) {
        // just move it to the top of the list
        std::swap(*pos, *m_recentSnapshots.begin());
        Q_EMIT recentSnapshotsChanged(m_recentSnapshots);
        return;
    }

    // add it to the top of the list and trim the list if required
    m_recentSnapshots.insert(m_recentSnapshots.cbegin(), std::move(fileName));

    if (maxRecentSnapshots < m_recentSnapshots.size()) {
        m_recentSnapshots.erase(m_recentSnapshots.cbegin() + maxRecentSnapshots, m_recentSnapshots.cend());
    }

    Q_EMIT recentSnapshotsChanged(m_recentSnapshots);
}

void Application::removeRecentSnapshot(QString fileName)
{
    fileName = QFileInfo(fileName).absoluteFilePath();

    auto pos = std::find_if(m_recentSnapshots.begin(), m_recentSnapshots.end(), [&fileName] (const auto & recentSnapshot) -> bool {
        return QFileInfo(recentSnapshot).absoluteFilePath() == fileName;
    });

    if (pos == m_recentSnapshots.cend()) {
        // not in list, nothing to do
        return;
    }

    m_recentSnapshots.erase(pos);
    Q_EMIT recentSnapshotsChanged(m_recentSnapshots);
}

void Application::loadRecentSnapshots()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("application"));
    const auto vSnapshots = settings.value(QLatin1String("recentSnapshots"));

    if (!vSnapshots.canConvert<QStringList>()) {
        Util::debug << "found invalid list of recent snapshots in settings\n";
        settings.remove(QLatin1String("recentSnapshots"));
        settings.endGroup();
        return;
    }

    settings.endGroup();
    const auto snapshots = vSnapshots.value<QStringList>();
    m_recentSnapshots.clear();
    std::copy(snapshots.cbegin(), snapshots.cend(), std::back_inserter(m_recentSnapshots));
    Q_EMIT recentSnapshotsChanged(m_recentSnapshots);
}

void Application::saveRecentSnapshots() const
{
    QStringList snapshots;
    std::copy(m_recentSnapshots.cbegin(), m_recentSnapshots.cend(), std::back_inserter(snapshots));

    QSettings settings;
    settings.beginGroup(QLatin1String("application"));
    settings.setValue(QLatin1String("recentSnapshots"), snapshots);
    settings.endGroup();
}
