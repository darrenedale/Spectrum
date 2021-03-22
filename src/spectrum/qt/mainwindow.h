#ifndef SPECTRUM_EMULATOR_MAINWINDOW_H
#define SPECTRUM_EMULATOR_MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>

#include "../spectrum.h"
#include "qimagedisplaydevice.h"
#include "qinterfacetwojoystick.h"
#include "qkempstonjoystick.h"
#include "debugwindow.h"
#include "imagewidget.h"
#include "keyboard.h"
#include "thread.h"

namespace Spectrum::Qt
{
	class QImageDisplayDevice;

	class MainWindow
	:	public QMainWindow {

    Q_OBJECT

    public:
         explicit MainWindow(QWidget * = nullptr);
         ~MainWindow() override;

        void saveScreenshot(const QString & fileName);

        void loadSnapshot(const QString & fileName, QString format = {});
        void loadSpSnapshot(const QString & fileName);

        // NOTE can't be const because the thread must be paused
        void saveSnapshot(const QString & fileName, QString format = {});

        inline Spectrum & spectrum()
        {
            return m_spectrum;
        }

        inline const Spectrum & spectrum() const
        {
            return m_spectrum;
        }

        bool eventFilter(QObject *, QEvent *) override;

	protected:
	    static QString guessSnapshotFormat(const QString & fileName);
        void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;
	    void keyPressEvent(QKeyEvent *) override;
	    void keyReleaseEvent(QKeyEvent *) override;
	    void dragEnterEvent(QDragEnterEvent *) override;
	    void dropEvent(QDropEvent *) override;
        void refreshSpectrumDisplay();

        void updateStatusBarSpeedWidget();

    private:
        using Joystick = QKempstonJoystick;

        void createToolbars();
        void createStatusBar();
        void connectSignals();

        void pauseResumeTriggered();
        void saveScreenshotTriggered();
        void loadSnapshotTriggered();
        void saveSnapshotTriggered();
        void emulationSpeedChanged(int);
        void stepTriggered();
        void debugTriggered();

        void threadPaused();
        void threadResumed();
        void threadStepped();

        void debugWindowHidden();
        void debugWindowShown();

        QString m_lastSnapshotLoadDir;
        QString m_lastScreenshotDir;
        Spectrum m_spectrum;
        Thread m_spectrumThread;
        Keyboard m_keyboard;
        QImageDisplayDevice m_display;
        ImageWidget m_displayWidget;
        QAction m_load;
        QAction m_save;
        QAction m_pauseResume;
        QAction m_refreshScreen;
        QAction m_saveScreenshot;
        QAction m_reset;
        QAction m_debug;
        QAction m_debugStep;
        QSlider m_emulationSpeedSlider;
        QSpinBox m_emulationSpeedSpin;
        DebugWindow m_debugWindow;

        // status bar widgets
        QLabel m_statusBarPause;
        QLabel m_statusBarEmulationSpeed;
        QLabel m_statusBarMHz;

        QTimer m_displayRefreshTimer;
        Joystick m_joystick;
	};
}

#endif // SPECTRUM_EMULATOR_MAINWINDOW_H
