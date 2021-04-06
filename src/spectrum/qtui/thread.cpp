#include <iostream>

#include <QMutexLocker>
#include <QApplication>
#include <QDebug>

#include "../basespectrum.h"
#include "thread.h"

namespace
{
    constexpr const int DefaultInstructionCount = 50;
}

using namespace Spectrum::QtUi;

Thread::Thread(BaseSpectrum & spectrum, QObject * parent )
: QThread(parent),
    m_threadLock(),
    m_waitCondition(),
	m_spectrum(&spectrum),
	m_pause(false),
	m_quit(false),
	m_step(false),
	m_debugMode(false)
{}

Thread::~Thread()
{
    m_quit = true;
    m_pause = false;
    m_debugMode = false;
    m_reset = false;
    m_threadLock.lock();
    m_waitCondition.wakeAll();
    m_threadLock.unlock();
    wait();
    m_spectrum = nullptr;
}

void Thread::run()
{
	m_quit = false;
	m_pause = false;

    while (!m_quit) {
        if (m_pause && !m_step) {
            QMutexLocker locker(&m_threadLock);
            m_waitCondition.wait(&m_threadLock);
        }

        if (m_reset) {
            m_spectrum->reset();
            m_reset = false;
            continue;
        }

        m_spectrum->run(((m_pause && m_step) || m_debugMode ? 1 : DefaultInstructionCount));

        if (m_step) {
            Q_EMIT stepped();
        }

        m_step = false;
    }
}

void Thread::pause()
{
    if (m_pause) {
        return;
    }

	m_pause = true;
    Q_EMIT paused();
}

void Thread::step()
{
	m_step = true;
    m_waitCondition.wakeAll();
}

void Thread::resume()
{
    if (!m_pause) {
        return;
    }

	m_pause = false;
	m_waitCondition.wakeAll();
    Q_EMIT resumed();
}

void Thread::reset()
{
    if (m_reset) {
        return;
    }

    m_reset = true;
    m_waitCondition.wakeAll();
}

void Thread::stop()
{
    m_pause = false;
    m_quit = true;
    m_waitCondition.wakeAll();
}

void Thread::setDebugMode(bool debug)
{
    if (debug == m_debugMode) {
        return;
    }

    m_debugMode = debug;

    if (debug) {
        Q_EMIT debuggingStarted();
    } else {
        Q_EMIT debuggingFinished();
    }
}

bool Thread::setSpectrum(Spectrum::BaseSpectrum & spectrum)
{
    if (isRunning()) {
        return false;
    }

    m_spectrum = &spectrum;
    Q_EMIT spectrumChanged(m_spectrum);
    return true;
}
