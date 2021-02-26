#ifndef SPECTRUM_EMULATOR_MAINWINDOW_H
#define SPECTRUM_EMULATOR_MAINWINDOW_H

#include <memory>
#include <QMainWindow>

#include "../spectrum.h"
#include "../qt/ui/qspectrumdisplay.h"

class QAction;
class QSlider;
class QSpinBox;
class ImageWidget;

namespace Spectrum
{
	class QSpectrumDisplay;
	class SpectrumThread;
	class QSpectrumDebugWindow;

	class MainWindow
	:	public QMainWindow {

    Q_OBJECT

    public:
         explicit MainWindow(QWidget * = nullptr);
         ~MainWindow() override;

        void saveSnapshot( const QString & fileName ) const;

        inline Spectrum & spectrum()
        {
            return m_spectrum;
        }

        inline const Spectrum & spectrum() const
        {
            return m_spectrum;
        }

	protected:
	    void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;

        void startPauseClicked();
        void snapshotTriggered();
        void updateSpectrumDisplay(const QImage &);

    private:
        void createWidgets();
        void connectWidgets();

        std::unique_ptr<SpectrumThread> m_spectrumThread;
        Spectrum m_spectrum;
        QSpectrumDisplay m_display;
        std::unique_ptr<ImageWidget> m_displayWidget;
        QAction * m_startPause;
        QAction * m_snapshot;
        QAction * m_reset;
        QAction * m_debug;
        QAction * m_debugStep;
        QSlider * m_emulationSpeedSlider;
        QSpinBox * m_emulationSpeedSpin;
        QSpectrumDebugWindow * m_debugWindow;
	};
}

#endif // SPECTRUM_EMULATOR_MAINWINDOW_H
