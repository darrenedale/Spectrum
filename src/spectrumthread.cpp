#include "spectrumthread.h"

#include <QMutexLocker>
#include <QApplication>
#include <QDebug>

#include "spectrum.h"

#define SPECTRUM_RUN_DEFAULT_INSTRUCTION_COUNT 50

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

void SpectrumThread::run()
{
	int instructionCount;
	m_quit = false;
	m_pause = false;
    m_spectrum.reset();

    while (!m_quit) {
        QApplication::processEvents();

        if (m_pause || (m_debugMode && !m_step)) {
            msleep(20);
            continue;
        }

        if (m_debugMode) {
            instructionCount = 1;
        } else {
            instructionCount = SPECTRUM_RUN_DEFAULT_INSTRUCTION_COUNT;
        }

        m_spectrumLock.lock();
        m_spectrum.run(instructionCount);
        m_spectrumLock.unlock();

        if (m_debugMode) {
            Q_EMIT debugStepTaken();
            m_step = false;
        }
    }
}

void SpectrumThread::pause()
{
	m_pause = true;
}

void SpectrumThread::setDebugMode(bool on)
{
	m_debugMode = on;
}

void SpectrumThread::step()
{
	m_step = true;
}

void SpectrumThread::resume()
{
	m_pause = false;
}

void SpectrumThread::reset()
{
	QMutexLocker locker(&m_spectrumLock);
	m_spectrum.reset();
}

void SpectrumThread::quit()
{
	m_pause = false;
	m_quit = true;
}
