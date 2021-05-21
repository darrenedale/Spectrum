//
// Created by darren on 25/02/2021.
//

#ifndef SPECTRUM_EMULATOR_APPLICATION_H
#define SPECTRUM_EMULATOR_APPLICATION_H

#define spectrumApp (::Spectrum::QtUi::Application::instance())

#include <memory>
#include <vector>
#include <QApplication>

namespace Spectrum::QtUi
{
    class MainWindow;

    class Application : public QApplication
    {
        Q_OBJECT

    public:
        using RecentSnapshots = std::vector<QString>;

        /**
         * Types of theme.
         *
         * The application object will attempt to determine whether the current style is a light or dark theme based on the standard window background colour.
         */
        enum class ThemeType
        {
            Unknown = 0,
            Light,
            Dark,
        };

        /**
         * Create a new instance of the emulator application.
         *
         * The application creates and opens a main window.
         *
         * @param argc A reference to the count of args provided to main()
         * @param argv The args provided to main()
         */
        Application(int & argc, char ** argv);

        /**
         * Destructor.
         *
         * The destructor is default, but must be declared and defaulted in the implementation file so that the instantiation of a unique_ptr with a forward-
         * declared class (MainWindow) compiles correctly.
         */
        ~Application() override;

        /**
         * Fetch a read-only reference to the emulator main window.
         *
         * @return The main window.
         */
        [[nodiscard]] const MainWindow & mainWindow() const;

        /**
         * Fetch a reference to the emulator main window.
         *
         * @return The main window.
         */
        MainWindow & mainWindow();

        /**
         * Fetch the most recently-loaded snapshots.
         *
         * The list could be empty. The returned list is ordered most-recently-used to least-recently-used.
         *
         * @return The set of snapshots.
         */
        [[nodiscard]] const RecentSnapshots & recentSnapshots() const
        {
            return m_recentSnapshots;
        }

        /**
         * Add a snapshot file to the list of recently-loaded snapshots.
         *
         * If the snapshot is already in the list it won't be added but will be moved to the top of the list (i.e. the most recently used).
         *
         * @param fileName The path to the snapshot to add.
         */
        void addRecentSnapshot(QString fileName);

        /**
         * Remove a snapshot file from the list of recently-loaded snapshots.
         *
         * If the snapshot is not in the list this is a no-op.
         *
         * @param fileName The path to the snapshot to remove.
         */
        void removeRecentSnapshot(QString fileName);

        /**
         * Clear the list of recently-loaded snapshots.
         */
        void clearRecentSnapshots()
        {
            if (m_recentSnapshots.empty()) {
                return;
            }

            m_recentSnapshots.clear();
            Q_EMIT recentSnapshotsChanged(m_recentSnapshots);
        }

        /**
         * Show an application-wide notification.
         *
         * Shows a desktop notification. If desktop notifications are not available, shows a dialogue box.
         *
         * @param message The message to show.
         * @param timeout How long the message should be displayed for.
         */
        static void showNotification(const QString & message, int timeout = 5000);

        /**
         * Fetch the Application singleton instance.
         *
         * @return
         */
        static Application * instance()
        {
            return m_instance;
        }

        /**
         * Try to determine whether the current theme is dark or light in nature.
         *
         * The lightness of underlying colour for the general window background brush is used to determine this. Use it to determine which types of icon to use,
         * for example, when there are different options for dark and light themes.
         *
         * @return The type of theme.
         */
        [[nodiscard]] static ThemeType themeType() ;

        /**
         * Fetch a named icon.
         *
         * @param name The icon to fetch.
         * @param type The theme type for the icon. Default is Unknown, which will use the current system theme type.
         *
         * @return The icon. This will be a null icon if the provided name does not name a valid icon.
         */
        [[nodiscard]] static QIcon icon(const QString & name, ThemeType type = ThemeType::Unknown) ;

        /**
         * Fetch a theme icon, using a named icon as fallback.
         *
         * The icon will be fetched from the system theme, or will use the built-in icon if the platform doesn't have a concept of system-wide icon themes or
         * there is no matching icon in the theme.
         *
         * @param systemThemeName The icon to use from the system theme, if possible.
         * @param name The built-in icon to use as a fallback.
         * @param type The theme type for the built-in icon, if required. Default is Unknown, which will use the current system theme type.
         *
         * @return The icon. This will be a null icon if the provided name does not name a valid icon.
         */
        [[nodiscard]] static QIcon icon(const QString & systemThemeName, const QString & name, ThemeType type = ThemeType::Unknown) ;

    Q_SIGNALS:
        /**
         * Emitted whenever the list of recent snapshots has changed.
         *
         * @param snapshots The list of recent snapshots.
         */
        void recentSnapshotsChanged(const RecentSnapshots & snapshots);

    protected:
        /**
         * Read the list of recent snapshots from the settings.
         */
        void loadRecentSnapshots();

        /**
         * Save the list of recent snapshots to the settings.
         */
        void saveRecentSnapshots() const;

    private:
        /**
         * The application singleton instance.
         */
        static Application * m_instance;

        /**
         * The main window.
         */
        std::unique_ptr<MainWindow> m_mainWindow;

        /**
         * The most recently-loaded snapshots.
         */
        RecentSnapshots m_recentSnapshots;
    };
}

#endif //SPECTRUM_EMULATOR_APPLICATION_H
