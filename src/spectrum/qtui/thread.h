#ifndef SPECTRUM_QTUI_THREAD_H
#define SPECTRUM_QTUI_THREAD_H

#include <print>

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "../../util/assert.h"

namespace Spectrum
{
    class BaseSpectrum;
}

namespace Spectrum::QtUi
{
	class Thread
	:	public QThread
	{
        Q_OBJECT

        public:
            explicit Thread(BaseSpectrum &, QObject * parent = nullptr);
            ~Thread() override;

	        [[nodiscard]]
            const BaseSpectrum & spectrum() const
            {
                sp_assert(m_spectrum, "detected null spectrum in thread in Thread::spectrum()");
                return *m_spectrum;
            }

	        [[nodiscard]]
            BaseSpectrum & spectrum()
            {
                sp_assert(m_spectrum, "detected null spectrum in thread in Thread::spectrum()");
                return *m_spectrum;
            }

	        [[nodiscard]]
            bool isPaused() const noexcept
            {
                return m_pause;
            }

	        [[nodiscard]]
            bool isInDebugMode() const
            {
                return m_debugMode;
            }

            bool setSpectrum(BaseSpectrum & spectrum);

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
            void spectrumReset();
            void spectrumChanged(Spectrum::BaseSpectrum *);

        protected:
            void run() override;

        private:
            QMutex m_threadLock;
            QWaitCondition m_waitCondition;
            BaseSpectrum * m_spectrum;
            bool m_pause;
            bool m_quit;
            bool m_reset;
            bool m_step;
            bool m_debugMode;
	};
}

#endif // SPECTRUM_QTUI_THREAD_H
