#ifndef SPECTRUM_QT_THREAD_H
#define SPECTRUM_QT_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace Spectrum
{
    class Spectrum;
}

namespace Spectrum::Qt
{
	class Thread
	:	public QThread {

		Q_OBJECT

		public:
			explicit Thread(Spectrum &, QObject * parent = nullptr);
			~Thread() override;

			inline const Spectrum & spectrum() const
			{
				return m_spectrum;
			}

			inline Spectrum & spectrum()
			{
				return m_spectrum;
			}

			inline bool isPaused() const
            {
			    return m_pause;
            }

			inline bool isInDebugMode() const
            {
			    return m_debugMode;
            }

		Q_SIGNALS:
	        void paused();
	        void resumed();
			void stepped();
			void debuggingStarted();
			void debuggingFinished();

		public Q_SLOTS:
			void setDebugMode(bool debug = true);
			void pause();
			void reset();
			void resume();
			void quit();
			void step();

		protected:
			void run() override;

		private:
			QMutex m_spectrumLock;
			QMutex m_threadLock;
			QWaitCondition m_waitCondition;
			Spectrum & m_spectrum;
			bool m_pause;
			bool m_quit;
			bool m_step;
			bool m_debugMode;
	};
}

#endif // SPECTRUM_QT_THREAD_H
