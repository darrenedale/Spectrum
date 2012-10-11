#ifndef SPECTRUMTHREAD_H
#define SPECTRUMTHREAD_H

#include <QThread>
#include <QMutex>

namespace Spectrum {
	class Spectrum;

	class SpectrumThread
	:	public QThread {

		Q_OBJECT

		public:
			explicit SpectrumThread( Spectrum * speccy = 0, QObject * parent = 0 );

			inline Spectrum * spectrum( void ) const {
				return m_spectrum;
			}

		signals:
			void debugStepTaken( void );

		public slots:
			void pause( void );
			void reset( void );
			void resume( void );
			void quit( void );
			void setDebugMode( bool on );
			void step( void );

		protected:
			virtual void run( void );

		private:
			QMutex m_spectrumLock;
			Spectrum * m_spectrum;
			bool m_pause, m_quit, m_debugMode, m_step;
	};
}

#endif // SPECTRUMTHREAD_H
