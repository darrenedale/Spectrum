#ifndef SPECTRUM_QTUI_THREAD_H
#define SPECTRUM_QTUI_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

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

        inline const BaseSpectrum & spectrum() const
        {
            return m_spectrum;
        }

        inline BaseSpectrum & spectrum()
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
        BaseSpectrum & m_spectrum;
        bool m_pause;
        bool m_quit;
        bool m_reset;
        bool m_step;
        bool m_debugMode;
	};
}

#endif // SPECTRUM_QTUI_THREAD_H
