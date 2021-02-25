//
// Created by darren on 25/02/2021.
//

#ifndef SCREENVIEW_MAINWINDOW_H
#define SCREENVIEW_MAINWINDOW_H

#include <cstdint>
#include <memory>
#include <QMainWindow>
#include <QSettings>

#include "../qspectrumdisplay.h"

class QLineEdit;
class QPushButton;

namespace ScreenView
{
    class MainWindow
    : public QMainWindow
    {
    public:
        MainWindow(QWidget * = nullptr);
        ~MainWindow() override;

        void loadScreen(const QString & fileName);
        void chooseScreenFile();

    protected:
        void showEvent(QShowEvent *) override;
        void closeEvent(QCloseEvent *) override;

    private:
        // 6144 for pixel data, 768 for attrs
        static const std::size_t DisplayFileSize = 6912;
        typedef std::uint8_t ScreenData[DisplayFileSize];

        std::unique_ptr<QLineEdit> m_fileName;
        std::unique_ptr<QPushButton> m_chooseFile;
        std::unique_ptr<Spectrum::QSpectrumDisplay> m_display;
        ScreenData m_screenData;
    };
}

#endif //SCREENVIEW_MAINWINDOW_H
