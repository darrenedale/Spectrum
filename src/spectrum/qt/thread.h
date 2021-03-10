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

		Q_SIGNALS:
	        void paused();
	        void resumed();
			void stepped();

		public Q_SLOTS:
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
	};
}

#endif // SPECTRUM_QT_THREAD_H
