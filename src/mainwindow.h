#ifndef SPECTRUM_MAINWINDOW_H
#define SPECTRUM_MAINWINDOW_H

#include <memory>
#include <QMainWindow>

#include "spectrum.h"

class QAction;
class QSlider;
class QSpinBox;

namespace Spectrum {
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

			inline Spectrum & spectrum() {
				return m_spectrum;
			}

			inline const Spectrum & spectrum() const {
				return m_spectrum;
			}

	protected:
	    void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;

		protected Q_SLOTS:
			void startPauseClicked();
			void snapshotTriggered();

		private:
			void createWidgets();
			void connectWidgets();

			std::unique_ptr<SpectrumThread> m_spectrumThread;
			Spectrum m_spectrum;
			QSpectrumDisplay * m_display;
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

#endif // SPECTRUM_MAINWINDOW_H
