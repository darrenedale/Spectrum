#ifndef SPECTRUM_EMULATOR_MAINWINDOW_H
#define SPECTRUM_EMULATOR_MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>

#include "../spectrum48k.h"
#include "qimagedisplaydevice.h"
#include "qinterfacetwojoystick.h"
#include "qkempstonjoystick.h"
#include "debugwindow.h"
#include "imagewidget.h"
#include "pokeswidget.h"
#include "keyboard.h"
#include "thread.h"

namespace Spectrum::Qt
{
	class QImageDisplayDevice;

	/**
	 * Spectrum emulator main window.
	 */
	class MainWindow
	: public QMainWindow
	{

    Q_OBJECT

    public:
         explicit MainWindow(QWidget * = nullptr);
         ~MainWindow() override;

        void saveScreenshot(const QString & fileName);

        /**
         * Load a snapshot from a give file.
         *
         * If the format is provided, the file is expected to be in that format. Otherwise the format will be determined
         * from the file name extension. The following formats are supported:
         * "z80" - files in Z80 format (https://worldofspectrum.org/faq/reference/z80format.htm)
         * "sna" - files in SNA format (https://worldofspectrum.org/faq/reference/formats.htm#File)
         * "sp"  - files in SP format
         * "zx"  - files in ZX format
         *
         * @param fileName
         * @param format
         */
        void loadSnapshot(const QString & fileName, QString format = {});
        void loadSnapshotFromSlot(int slotIndex);

        // NOTE can't be const because the thread must be paused
        void saveSnapshot(const QString & fileName, QString format = {});
        void saveSnapshotToSlot(int slotIndex, QString format = {});

        void loadPokes(const QString & fileName);

        inline Spectrum48k & spectrum()
        {
            return m_spectrum;
        }

        [[nodiscard]] inline const Spectrum48k & spectrum() const
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

        void createMenuBar();
        void createToolBars();
        void createDockWidgets();
        void createStatusBar();
        void connectSignals();

        void pauseResumeTriggered();
        void saveScreenshotTriggered();
        void loadSnapshotTriggered();
        void saveSnapshotTriggered();
        void loadPokesTriggered();
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
        QString m_lastPokeLoadDir;
        Spectrum48k m_spectrum;
        Thread m_spectrumThread;
        Keyboard m_keyboard;
        QImageDisplayDevice m_display;
        ImageWidget m_displayWidget;
        PokesWidget m_pokesWidget;
        QAction m_load;
        QAction m_save;
        QAction m_pauseResume;
        QAction m_refreshScreen;
        QAction m_saveScreenshot;
        QAction m_colourDisplay;
        QAction m_monochromeDisplay;
        QAction m_bwDisplay;

        QAction m_joystickKempston;
        QAction m_joystickInterface2;

        QAction m_loadPokes;

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
