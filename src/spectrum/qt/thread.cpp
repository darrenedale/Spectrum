#include "thread.h"

#include <QMutexLocker>
#include <QApplication>
#include <QDebug>

#include "../spectrum.h"

namespace
{
    constexpr const int DefaultInstructionCount = 50;
}

using namespace Spectrum::Qt;

Thread::Thread(Spectrum & spectrum, QObject * parent )
:	QThread(parent),
	m_spectrum(spectrum),
	m_pause(false),
	m_quit(false),
	m_step(false)
{
}

Thread::~Thread()
{
    m_threadLock.lock();
    m_quit = true;
    m_pause = false;
    m_waitCondition.wakeOne();
    m_threadLock.unlock();
    wait();
}

void Thread::run()
{
    m_threadLock.lock();
	m_quit = false;
	m_pause = false;
	m_threadLock.unlock();

	m_spectrumLock.lock();
    m_spectrum.reset();
	m_spectrumLock.unlock();

    while (!m_quit) {
        if (m_pause && !m_step) {
            m_threadLock.lock();
            m_waitCondition.wait(&m_threadLock);
            m_threadLock.unlock();
        }

        m_spectrumLock.lock();
        m_spectrum.run((m_pause && m_step ? 1 : DefaultInstructionCount));
        m_spectrumLock.unlock();

        if (m_step) {
            Q_EMIT stepped();
        }

        m_step = false;
    }
}

void Thread::pause()
{
    QMutexLocker locker(&m_threadLock);
	m_pause = true;
    Q_EMIT paused();
}

void Thread::step()
{
    QMutexLocker locker(&m_threadLock);
	m_step = true;
    m_waitCondition.wakeOne();
}

void Thread::resume()
{
    QMutexLocker locker(&m_threadLock);
	m_pause = false;
	m_waitCondition.wakeOne();
    Q_EMIT resumed();
}

void Thread::reset()
{
	QMutexLocker locker(&m_spectrumLock);
	m_spectrum.reset();
}

void Thread::quit()
{
    QMutexLocker locker(&m_threadLock);
    m_pause = false;
    m_quit = true;
    m_waitCondition.wakeOne();
}
