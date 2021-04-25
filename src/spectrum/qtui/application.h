//
// Created by darren on 25/02/2021.
//

#ifndef SPECTRUM_EMULATOR_APPLICATION_H
#define SPECTRUM_EMULATOR_APPLICATION_H

#define spectrumApp (::Spectrum::QtUi::Application::instance())

#include <memory>
#include <QApplication>
#include "mainwindow.h"

namespace Spectrum::QtUi
{
    class Application : public QApplication
    {
        Q_OBJECT

    public:
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
         * Fetch a read-only reference to the emulator main window.
         *
         * @return The main window.
         */
        [[nodiscard]] const MainWindow & mainWindow() const
        {
            return *m_mainWindow;
        }

        /**
         * Fetch a reference to the emulator main window.
         *
         * @return The main window.
         */
        MainWindow & mainWindow()
        {
            return *m_mainWindow;
        }

        /**
         * Show an application-wide notification.
         *
         * Currently these go to the status bar of the main window, but the will be moved at some point to a desktop notification.
         *
         * @param message The message to show.
         * @param timeout How long the message should be displayed for.
         */
        void showMessage(const QString & message, int timeout = 5000);

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
        [[nodiscard]] ThemeType themeType() const;

        /**
         * Fetch a named icon.
         *
         * The icon will be suitable for the current theme type.
         *
         * @param name The icon to fetch.
         *
         * @return The icon. This will be a null icon if the provided name does not name a valid icon.
         */
        [[nodiscard]] QIcon icon(const QString & name) const;

    private:
        /**
         * The application singleton instance.
         */
        static Application * m_instance;

        /**
         * The main window.
         */
        std::unique_ptr<MainWindow> m_mainWindow;
    };
}

#endif //SPECTRUM_EMULATOR_APPLICATION_H
