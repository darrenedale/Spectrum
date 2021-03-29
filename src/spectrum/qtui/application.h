//
// Created by darren on 25/02/2021.
//

#ifndef SPECTRUM_EMULATOR_APPLICATION_H
#define SPECTRUM_EMULATOR_APPLICATION_H

#include <QApplication>
#include "mainwindow.h"

namespace Spectrum::QtUi
{
    class Application : public QApplication
    {
        Q_OBJECT

    public:
        Application(int & argc, char ** argv);

        [[nodiscard]] const MainWindow & mainWindow() const
        {
            return m_mainWindow;
        }

        MainWindow & mainWindow()
        {
            return m_mainWindow;
        }

        void showMessage(const QString & message, int timeout = 5000);

        static Application * instance()
        {
            return m_instance;
        }

    private:
        static Application * m_instance;
        MainWindow m_mainWindow;
    };
}

#endif //SPECTRUM_EMULATOR_APPLICATION_H
