#ifndef SPECTRUM_QTUI_MAINWINDOW_H
#define SPECTRUM_QTUI_MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include "qimagedisplaydevice.h"
#include "debugger/debugwindow.h"
#include "aboutwidget.h"
#include "helpwidget.h"
#include "spectrumdisplayimagewidget.h"
#include "cheatsview.h"
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
	    /**
	     * Default-initialise a new MainWindow.
	     *
	     * @param parent The widget that owns the new MainWindow instance.
	     */
        explicit MainWindow(QWidget * parent = nullptr);

        /**
         * MainWindow objects cannot be copy-constructed.
         */
        MainWindow(const MainWindow &) = delete;

        /**
         * MainWindow objects cannot be move-constructed.
         */
        MainWindow(MainWindow &&) = delete;

        /**
         * MainWindow objects cannot be copy assigned.
         */
        void operator=(const MainWindow &) = delete;

        /**
         * MainWindow objects cannot be move assigned.
         */
        void operator=(MainWindow &&) = delete;

        /**
         * Destructor.
         */
        ~MainWindow() override;

        /**
         * Switch the running Spectrum to the given model.
         *
         * The thread will be stopped, the currently running Spectrum will be destroyed and a new Spectrum of the provided model will be created and run by the
         * thread. This implies that any unsaved work or game progress in the previously running Spectrum will be discarded.
         *
         * @param model The model to switch to.
         */
        void setModel(Spectrum::Model model);

        /**
         * Fetch the model of the currently running Spectrum.
         *
         * @return One of the Spectrum::Model constants.
         */
        Spectrum::Model model() const;

        /**
         * Save a screenshot of the current Spectrum screen to the provided file.
         *
         * If the provided filename ends in .scr it is saved as a Spectrum screen dump. Otherwise, the extension indicates one of the formats supported by Qt.
         *
         * @param fileName The file to save to.
         */
        void saveScreenshot(const QString & fileName);

        /**
         * Load a snapshot from a give file.
         *
         * If the format is provided, the file is expected to be in that format. Otherwise the format will be determined
         * from the file size and content, or its filename extension. The following formats are supported:
         * "z80" - files in Z80 format (https://worldofspectrum.org/faq/reference/z80format.htm)
         * "sna" - files in SNA format (https://worldofspectrum.org/faq/reference/formats.htm#File)
         * "sp"  - files in SP format (http://spectrum-zx.chat.ru/faq/fileform.html)
         * "zx"  - files in ZX format (http://spectrum-zx.chat.ru/faq/fileform.html)
         * "zx82"  - files in ZX82 format (http://spectrum-zx.chat.ru/faq/fileform.html)
         *
         * @param fileName
         * @param format
         */
        bool loadSnapshot(const QString & fileName, QString format = {});

        /**
         * Load a snapshot from a given quick-save slot.
         *
         * @param slotIndex The slot to load from.
         *
         * @return true if the snapshot was loaded, false otherwise.
         */
        bool loadSnapshotFromSlot(int slotIndex);

        // NOTE can't be const because the thread must be paused
        void saveSnapshot(const QString & fileName, QString format = {});

        /**
         * Save a snapshot to a given quick-save slot.
         *
         * @param slotIndex The slot to save to.
         * @param format Set the save format. If an empty string is provided, the default format (currently .z80) will be used.
         */
        void saveSnapshotToSlot(int slotIndex, QString format = {});

        /**
         * Rescan the game controllers connected to the host.
         *
         * The controllers menu is re-populated. This is called automatically if a controller is disconnected or a new controller is connected.
         */
        void rescanGameControllers();

        /**
         * Fetch a read/write reference to the currently running Spectrum.
         *
         * @return The spectrum.
         */
        inline BaseSpectrum & spectrum()
        {
            return *m_spectrum;
        }

        /**
         * Fetch a read-only reference to the currently running Spectrum.
         *
         * @return The spectrum.
         */
        [[nodiscard]] inline const BaseSpectrum & spectrum() const
        {
            return *m_spectrum;
        }

        /**
         * Fetch a read/write reference to the Spectrum thread.
         *
         * @return The thread.
         */
        inline Thread & spectrumThread()
        {
            return m_spectrumThread;
        }

        /**
         * Fetch a read-only reference to the Spectrum thread.
         *
         * @return The thread.
         */
        [[nodiscard]] inline const Thread & spectrumThread() const
        {
            return m_spectrumThread;
        }

        /**
         * Pause the currently running Spectrum.
         */
        inline void pauseSpectrum()
        {
            m_spectrumThread.pause();
        }

        /**
         * Resume the currently running Spectrum if it is paused.
         */
        inline void resumeSpectrum()
        {
            m_spectrumThread.resume();
        }

        bool eventFilter(QObject *, QEvent *) override;

	protected:
	    /**
	     * Helper to guess the format of a snapshot file.
	     *
	     * The returned strings come from the snapshot readers. The first reader that reports it can read the provided file is queried for its snapshot type
	     * string, which is returned.
	     *
	     * @param fileName The file to inspect.
	     *
	     * @return A string representation of the format. This will be empty if the format could not be reliably guessed.
	     */
	    static QString guessSnapshotFormat(const QString & fileName);

	    /**
	     * Helper to load the user's emulator settings.
	     */
	    void loadSettings();

        /**
         * Helper to load the user's current emulator settings.
         */
	    void saveSettings();

	    /**
	     * Handler for when the main window is made visible.
	     *
	     * Loads the last known state of the window (geometry, docks, toolbars, etc.).
	     */
        void showEvent(QShowEvent *) override;

        /**
         * Handler for when the main window is closed.
         *
         * Stores the state of the window (geometry, docks, toolbars, etc.).
         */
	    void closeEvent(QCloseEvent *) override;

	    /**
	     * Handler for when the window receives key presses.
	     *
	     * Along with keyReleaseEvent, handles the user holding down the TAB key to temporarily accelerate the emulation.
	     */
	    void keyPressEvent(QKeyEvent *) override;

	    /**
	     * Handler for when the window receives a key release event.
	     *
	     * Along with keyReleaseEvent, handles the user holding down the TAB key to temporarily accelerate the emulation.
	     */
	    void keyReleaseEvent(QKeyEvent *) override;

	    /**
	     * Handler for when the user drags some content into the window.
	     *
	     * Enables drag-and-drop loading of snapshot files.
	     */
	    void dragEnterEvent(QDragEnterEvent *) override;

        /**
         * Handler for when the user drops some content into the window.
         *
         * Loads a snapshot if it is dropped on the window.
         */
	    void dropEvent(QDropEvent *) override;

	    /**
	     * Helper to force a re-rendering of the Spectrum display to the UI.
	     */
        void refreshSpectrumDisplay();

        /**
         * Helper to synchronise the speed readout in the status bar with the running Spectrum.
         */
        void updateStatusBarSpeedWidget();

    private:
	    /**
	     * Helper to attach the devices the user has enabled to the Spectrum currently running.
	     *
	     * This is used when the model is changed to attach the devices to the new Spectrum model.
	     */
	    void attachSpectrumDevices();

        /**
         * Helper to remove the devices the user has enabled from the Spectrum currently running.
         *
         * This is used when the model is changed to remove the devices from the old Spectrum model before it is destroyed.
         */
	    void detachSpectrumDevices();

	    /**
	     * Helper to stop the Spectrum thread.
	     *
	     * The thread is stopped cleanly if possible. If the thread does not cleanly stop after 3 seconds it is forcibly terminated.
	     *
	     * This is used when the model is changed and in the destructor when the thread is being destroyed. In debug builds, console output will indicate how
	     * the thread stopped.
	     */
        void stopThread();

        /**
         * Helper to create the main window's menu bar.
         *
         * This is extracted to a helper primarily for ease of maintenance.
         */
        void createMenuBar();
        
        /**
         * Helper to create the File menu in the menu bar.
         */
        void createFileMenu();

        /**
         * Helper to create the Spectrum menu in the menu bar.
         */
        void createSpectrumMenu();

        /**
         * Helper to create the Display menu in the menu bar.
         */
        void createDisplayMenu();

        /**
         * Helper to create the Debugger menu in the menu bar.
         */
        void createDebuggerMenu();

        /**
         * Helper to create the Help menu in the menu bar.
         */
        void createHelpMenu();

        /**
         * Helper to create the main window's tool bars.
         *
         * This is extracted to a helper primarily for ease of maintenance.
         */
        void createToolBars();

        /**
         * Helper to create the main tool bar.
         */
        void createMainToolBar();

        /**
         * Helper to create the debug tool bar.
         */
        void createDebugToolBar();

        /**
         * Helper to create the speed tool bar.
         */
        void createSpeedToolBar();

        /**
         * Helper to create the main window's dock widgets.
         *
         * This is extracted to a helper primarily for ease of maintenance.
         */
        void createDockWidgets();

        /**
         * Helper to create the main window's status bar.
         *
         * This is extracted to a helper primarily for ease of maintenance.
         */
        void createStatusBar();

        /**
         * Helper to connect signals from the internal components.
         *
         * This is extracted to a helper primarily for ease of maintenance.
         */
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

        /**
         * Handler for when the Spectrum thread pauses.
         */
        void threadPaused();

        /**
         * Handler for when the Spectrum thread resumes after a pause.
         */
        void threadResumed();

        /**
         * Handler for when the Spectrum thread steps a single instruction while paused.
         */
        void threadStepped();

        QString m_lastSnapshotLoadDir;
        QString m_lastScreenshotDir;
        std::unique_ptr<BaseSpectrum> m_spectrum;
        QString m_lastPokeLoadDir;
        Thread m_spectrumThread;
        Keyboard m_keyboard;
        QImageDisplayDevice m_display;
        SpectrumDisplayImageWidget m_displayWidget;
        CheatsView m_pokesWidget;
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
        QActionGroup m_frameSkipGroup;
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
        Debugger::DebugWindow m_debugWindow;
        std::unique_ptr<AboutWidget> m_aboutWidget;
        std::unique_ptr<HelpWidget> m_helpWidget;

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

#endif // SPECTRUM_QTUI_MAINWINDOW_H
