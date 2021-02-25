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
			 MainWindow(QWidget * = nullptr);
			 virtual ~MainWindow();

			void saveSnapshot( const QString & fileName ) const;

			inline Spectrum & spectrum( void ) {
				return m_spectrum;
			}

	protected:
	    void closeEvent(QCloseEvent *) override;

		protected Q_SLOTS:
			void slotStartPause(bool);
			void slotSnapshot();

		private:
			void createWidgets();
			void connectWidgets();

			std::unique_ptr<SpectrumThread> m_spectrumThread;
			Spectrum m_spectrum;
			QSpectrumDisplay * m_display;
			QAction * m_startPause, * m_snapshot, * m_reset, * m_debug, * m_debugStep;
			QSlider * m_emulationSpeedSlider;
			QSpinBox * m_emulationSpeedSpin;
			QSpectrumDebugWindow * m_debugWindow;
	};
}

#endif // SPECTRUM_MAINWINDOW_H
