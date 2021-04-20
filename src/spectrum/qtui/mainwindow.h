#ifndef SPECTRUM_EMULATOR_MAINWINDOW_H
#define SPECTRUM_EMULATOR_MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include "qimagedisplaydevice.h"
#include "debugwindow.h"
#include "imagewidget.h"
#include "pokeswidget.h"
#include "../mouseinterface.h"
#include "../joystickinterface.h"
#include "../keyboard.h"
#include "gamecontrollerhandler.h"
#include "thread.h"

namespace Spectrum
{
    class BaseSpectrum;
}

namespace Spectrum::QtUi
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

        void setModel(Spectrum::Model model);
        Spectrum::Model model() const;

        void saveScreenshot(const QString & fileName);

        /**
         * Load a snapshot from a give file.
         *
         * If the format is provided, the file is expected to be in that format. Otherwise the format will be determined
         * from the file name extension. The following formats are supported:
         * "z80" - files in Z80 format (https://worldofspectrum.org/faq/reference/z80format.htm)
         * "sna" - files in SNA format (https://worldofspectrum.org/faq/reference/formats.htm#File)
         * "sp"  - files in SP format (http://spectrum-zx.chat.ru/faq/fileform.html)
         * "zx"  - files in ZX format (http://spectrum-zx.chat.ru/faq/fileform.html)
         *
         * @param fileName
         * @param format
         */
        bool loadSnapshot(const QString & fileName, QString format = {});
        bool loadSnapshotFromSlot(int slotIndex);

        // NOTE can't be const because the thread must be paused
        void saveSnapshot(const QString & fileName, QString format = {});
        void saveSnapshotToSlot(int slotIndex, QString format = {});

        /**
         * Rescan the game controllers connected to the host.
         *
         * The controllers menu is re-populated. This is called automatically if a controller is disconnected or a new controller is connected.
         */
        void rescanGameControllers();

        inline BaseSpectrum & spectrum()
        {
            return *m_spectrum;
        }

        [[nodiscard]] inline const BaseSpectrum & spectrum() const
        {
            return *m_spectrum;
        }

        inline Thread & spectrumThread()
        {
            return m_spectrumThread;
        }

        [[nodiscard]] inline const Thread & spectrumThread() const
        {
            return m_spectrumThread;
        }

        inline void pauseSpectrum()
        {
            m_spectrumThread.pause();
        }

        inline void resumeSpectrum()
        {
            m_spectrumThread.resume();
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
	    void attachSpectrumDevices();
	    void detachSpectrumDevices();
        void stopThread();

        void createMenuBar();
        void createToolBars();
        void createDockWidgets();
        void createStatusBar();
        void connectSignals();

        void pauseResumeTriggered();
        void saveScreenshotTriggered();
        void loadSnapshotTriggered();
        void saveSnapshotTriggered();
        void model16Triggered();
        void model48Triggered();
        void model128Triggered();
        void modelPlus2Triggered();
        void modelPlus2aTriggered();
        void modelPlus3Triggered();
        void useKempstonJoystickTriggered();
        void useInterfaceTwoJoystickTriggered();
        void useCursorJoystickTriggered();
        void useFullerJoystickTriggered();
        void noJoystickTriggered();
        void kempstonMouseToggled(bool);
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
        std::unique_ptr<BaseSpectrum> m_spectrum;
        QString m_lastPokeLoadDir;
        Thread m_spectrumThread;
        Keyboard m_keyboard;
        QImageDisplayDevice m_display;
        ImageWidget m_displayWidget;
        PokesWidget m_pokesWidget;
        QAction m_load;
        QAction m_save;

        QAction m_pauseResume;
        QAction m_reset;
        QAction m_model16;
        QAction m_model48;
        QAction m_model128;
        QAction m_modelPlus2;
        QAction m_modelPlus2a;
        QAction m_modelPlus3;

        QAction m_saveScreenshot;
        QAction m_colourDisplay;
        QAction m_monochromeDisplay;
        QAction m_bwDisplay;

        QAction m_joystickNone;
        QAction m_joystickKempston;
        QAction m_joystickInterface2;
        QAction m_joystickCursor;
        QAction m_joystickFuller;

        QMenu m_gameControllersMenu;
        QActionGroup m_gameControllersGroup;

        QAction m_kempstonMouse;

        QAction m_debug;
        QAction m_debugStep;
        QAction m_refreshScreen;

        QSlider m_emulationSpeedSlider;
        QSpinBox m_emulationSpeedSpin;
        DebugWindow m_debugWindow;

        // status bar widgets
        QLabel m_statusBarPause;
        QLabel m_statusBarEmulationSpeed;
        QLabel m_statusBarMHz;

        QTimer m_displayRefreshTimer;
        std::unique_ptr<JoystickInterface> m_joystick;
        GameControllerHandler m_gameControllerHandler;
        std::unique_ptr<MouseInterface> m_mouse;
	};
}

#endif // SPECTRUM_EMULATOR_MAINWINDOW_H
