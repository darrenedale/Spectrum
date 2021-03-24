#ifndef SPECTRUM_QT_THREAD_H
#define SPECTRUM_QT_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace Spectrum
{
    class Spectrum48k;
}

namespace Spectrum::Qt
{
	class Thread
	:	public QThread {

		Q_OBJECT

		public:
			explicit Thread(Spectrum48k &, QObject * parent = nullptr);
			~Thread() override;

			inline const Spectrum48k & spectrum() const
			{
				return m_spectrum;
			}

			inline Spectrum48k & spectrum()
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

			void setDebugMode(bool debug = true);
			void pause();
			void reset();
			void resume();
			void stop();
			void step();

		Q_SIGNALS:
	        void paused();
	        void resumed();
			void stepped();
			void debuggingStarted();
			void debuggingFinished();

		protected:
			void run() override;

		private:
			QMutex m_threadLock;
			QWaitCondition m_waitCondition;
			Spectrum48k & m_spectrum;
			bool m_pause;
			bool m_quit;
			bool m_reset;
			bool m_step;
			bool m_debugMode;
	};
}

#endif // SPECTRUM_QT_THREAD_H
