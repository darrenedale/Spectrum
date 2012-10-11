#include "spectrumthread.h"

#include "spectrum.h"
#include <QMutexLocker>
#include <QDebug>

#define SPECTRUM_RUN_DEFAULT_INSTRUCTION_COUNT 50

using namespace Spectrum;

SpectrumThread::SpectrumThread( Spectrum * speccy, QObject * parent )
:	QThread(parent),
	m_spectrum(speccy),
	m_pause(false),
	m_quit(false) {}


void SpectrumThread::run( void ) {
	int instructionCount;
	m_quit = m_pause = false;

	if(m_spectrum) {
		m_spectrum->reset();

		while(!m_quit) {
			if(m_debugMode) instructionCount = 1;
			else instructionCount = SPECTRUM_RUN_DEFAULT_INSTRUCTION_COUNT;

			m_spectrumLock.lock();
//			qDebug() << "running" << SPECTRUM_RUN_DEFAULT_INSTRUCTION_COUNT << "instructions";
			m_spectrum->run(instructionCount);
			m_spectrumLock.unlock();

			if(m_debugMode) {
				emit(debugStepTaken());

				/* if currently paused, wait for the step signal from the user */
				if(m_pause) while(!m_step) msleep(50);
				m_step = false;
			}
			else
				while(m_pause) msleep(50);
		}
	}
}



void SpectrumThread::pause( void ) {
	m_pause = true;
}


void SpectrumThread::setDebugMode( bool on ) {
	m_debugMode = on;
}


void SpectrumThread::step( void ) {
	m_step = true;
}


void SpectrumThread::resume( void ) {
	m_pause = false;
}


void SpectrumThread::reset( void ) {
	QMutexLocker locker(&m_spectrumLock);
	qDebug() << "resetting...";
	m_spectrum->reset();
}


void SpectrumThread::quit( void ) {
	m_pause = false;
	m_quit = true;
}
