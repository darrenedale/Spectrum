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
			explicit SpectrumThread(Spectrum &, QObject * parent = nullptr);

			inline const Spectrum & spectrum() const
			{
				return m_spectrum;
			}

			inline Spectrum & spectrum()
			{
				return m_spectrum;
			}

		Q_SIGNALS:
			void debugStepTaken();

		public Q_SLOTS:
			void pause();
			void reset();
			void resume();
			void quit();
			void setDebugMode(bool);
			void step();

		protected:
			void run() override;

		private:
			QMutex m_spectrumLock;
			Spectrum & m_spectrum;
			bool m_pause;
			bool m_quit;
			bool m_debugMode;
			bool m_step;
	};
}

#endif // SPECTRUMTHREAD_H
