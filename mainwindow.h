#ifndef SPECTRUM_MAINWINDOW_H
#define SPECTRUM_MAINWINDOW_H

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
			 MainWindow( QWidget * parent = 0 );
			 ~MainWindow( void );

			void saveSnapshot( const QString & fileName ) const;

			inline Spectrum & spectrum( void ) {
				return m_speccy;
			}

		protected slots:
			void slotStartPause( bool state );
			void slotSnapshot( void );

		private:
			void createWidgets( void );
			void connectWidgets( void );

			SpectrumThread * m_speccyThread;
			Spectrum m_speccy;
			QSpectrumDisplay * m_display;
			QAction * m_startPause, * m_snapshot, * m_reset, * m_debug, * m_debugStep;
			QSlider * m_emulationSpeedSlider;
			QSpinBox * m_emulationSpeedSpin;
			QSpectrumDebugWindow * m_debugWindow;
	};
}

#endif // SPECTRUM_MAINWINDOW_H
