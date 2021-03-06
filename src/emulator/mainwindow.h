#ifndef SPECTRUM_EMULATOR_MAINWINDOW_H
#define SPECTRUM_EMULATOR_MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>

#include "spectrum.h"
#include "../qt/ui/qspectrumdisplay.h"
#include "../qt/ui/qspectrumdebugwindow.h"
#include "../qt/ui/imagewidget.h"
#include "../qt/qspectrumkeyboard.h"
#include "../qt/spectrumthread.h"

namespace Spectrum
{
	class QSpectrumDisplay;

	class MainWindow
	:	public QMainWindow {

    Q_OBJECT

    public:
         explicit MainWindow(QWidget * = nullptr);
         ~MainWindow() override;

        void saveScreenshot(const QString & fileName) const;

        void loadSnapshot(const QString & fileName);
        void saveSnapshot(const QString & fileName) const;

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
        void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;
        void refreshSpectrumDisplay();

    private:
        void createToolbars();
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

        Spectrum m_spectrum;
        SpectrumThread m_spectrumThread;
        QSpectrumKeyboard m_keyboard;
        QSpectrumDisplay m_display;
        ImageWidget m_displayWidget;
        QAction m_load;
        QAction m_pauseResume;
        QAction m_refreshScreen;
        QAction m_screenshot;
        QAction m_reset;
        QAction m_debug;
        QAction m_debugStep;
        QSlider m_emulationSpeedSlider;
        QSpinBox m_emulationSpeedSpin;
        QSpectrumDebugWindow m_debugWindow;
        QTimer m_displayRefreshTimer;
	};
}

#endif // SPECTRUM_EMULATOR_MAINWINDOW_H
