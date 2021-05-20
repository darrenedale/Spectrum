//
// Created by darren on 25/02/2021.
//

#ifndef SPECTRUM_SCREENVIEW_MAINWINDOW_H
#define SPECTRUM_SCREENVIEW_MAINWINDOW_H

#include <cstdint>
#include <memory>
#include <array>
#include <QMainWindow>
#include <QSettings>
#include "../qtui/imagewidget.h"
#include "colourcombo.h"

class QLineEdit;
class QPushButton;

namespace Spectrum::ScreenView
{
    /**
     * The main window for the ScreenView app.
     */
    class MainWindow
    : public QMainWindow
    {
    public:
        /**
         * Initialise a new MainWindow.
         */
        explicit MainWindow(QWidget * = nullptr);

        /**
         * MainWindows cannot be copy-constructed.
         */
        MainWindow(const MainWindow &) = delete;

        /**
         * MainWindows cannot be move-constructed.
         */
        MainWindow(MainWindow &&) = delete;

        /**
         * MainWindows cannot be copy assigned.
         */
        void operator=(const MainWindow &) = delete;

        /**
         * MainWindows cannot be move assigned.
         */
        void operator=(MainWindow &&) = delete;

        /**
         * Destructor.
         */
        ~MainWindow() override;

        /**
         * Load a screen dump from a file.
         *
         * @param fileName The file to load.
         */
        void loadScreen(const QString & fileName);

        /**
         * Show a dialogue for the user to choose a screen dump file to load.
         */
        void chooseScreenFile();

    protected:
        /**
         * Handle show events.
         *
         * The last saved geometry for the window is loaded from the settings.
         */
        void showEvent(QShowEvent *) override;

        /**
         * Handle hide events.
         *
         * The geometry for the window is saved to the settings.
         */
        void closeEvent(QCloseEvent *) override;

        /**
         * Refresh the UI.
         */
        void updateScreen();

    private:
        // 6144 for pixel data, 768 for attrs
        static constexpr const std::size_t DisplayFileSize = 6912;
        using ScreenData = std::array<std::uint8_t, DisplayFileSize>;

        /**
         * Widget for the user to enter a file name.
         */
        std::unique_ptr<QLineEdit> m_fileName;

        /**
         * Widget for the user to trigger a file dialogue.
         */
        std::unique_ptr<QPushButton> m_chooseFile;

        /**
         * Widget for the user to choose the border colour.
         *
         * The border colour is not present in a screen dump.
         */
        std::unique_ptr<ColourCombo> m_borderColour;

        /**
         * The screen display widget.
         */
        std::unique_ptr<Spectrum::ImageWidget> m_display;

        /**
         * The screen data.
         */
        ScreenData m_screenData;
    };
}

#endif //SPECTRUM_SCREENVIEW_MAINWINDOW_H
