#include "spectrumthread.h"

#include <QMutexLocker>
#include <QApplication>
#include <QDebug>

#include "../emulator/spectrum.h"

namespace
{
    constexpr const int DefaultInstructionCount = 50;
}

using namespace Spectrum;

SpectrumThread::SpectrumThread(Spectrum & spectrum, QObject * parent )
:	QThread(parent),
	m_spectrum(spectrum),
	m_pause(false),
	m_quit(false),
	m_debugMode(false),
	m_step(false)
{
}

SpectrumThread::~SpectrumThread()
{
    m_threadLock.lock();
    m_quit = true;
    m_pause = false;
    m_waitCondition.wakeOne();
    m_threadLock.unlock();
    wait();
}

void SpectrumThread::run()
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

        if (m_debugMode) {
            Q_EMIT debugStepTaken();
        }

        m_step = false;
    }
}

void SpectrumThread::pause()
{
    QMutexLocker locker(&m_threadLock);
	m_pause = true;
}

void SpectrumThread::setDebugMode(bool on)
{
    QMutexLocker locker(&m_threadLock);
	m_debugMode = on;
}

void SpectrumThread::step()
{
    QMutexLocker locker(&m_threadLock);
	m_step = true;
    m_waitCondition.wakeOne();
}

void SpectrumThread::resume()
{
    QMutexLocker locker(&m_threadLock);
	m_pause = false;
	m_waitCondition.wakeOne();
}

void SpectrumThread::reset()
{
	QMutexLocker locker(&m_spectrumLock);
	m_spectrum.reset();
}

void SpectrumThread::quit()
{
    QMutexLocker locker(&m_threadLock);
    m_pause = false;
    m_quit = true;
    m_waitCondition.wakeOne();
}
