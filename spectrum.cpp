#include "spectrum.h"

#include "spectrumdisplaydevice.h"
#include "z80.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>


Spectrum::Spectrum::Spectrum( int memsize )
:	Computer(memsize) {
	init();
}


Spectrum::Spectrum::Spectrum( unsigned char * mem, int memsize )
:	Computer(mem, memsize) {
	init();
}


Spectrum::Spectrum::~Spectrum( void ) {
	/* base class takes care of RAM */
	Cpu * myCpu = cpu();
	removeCpu(myCpu);
	delete myCpu;
}


void Spectrum::Spectrum::init( void ) {
	Cpu * myCpu = new Z80(memory(), memorySize());
	/* 3.5MHz */
	myCpu->setClockSpeed(3500000);
	addCpu(myCpu);
}


void Spectrum::Spectrum::reset( void ) {
	/* clear RAM */
	for(int i = 0; i < memorySize(); ++i)
		m_ram[i] = 0;

	/* load ROM into lowest 16K of RAM */
	unsigned char * myMemory = memory();

	std::FILE * f = std::fopen("spectrum48.rom", "rb");

	if(!f) {
		std::cerr << "spectrum ROM file \"spectrum48.rom\" could not be opened.\n";
		return;
	}

	for(int i = 0; i < 16384; ++i) {
		if(feof(f)) {
			std::cerr << "failed to load spectrum ROM.\n";
			return;
		}

		myMemory[i] = fgetc(f);
	}

	refreshDisplays();

	/* fetch the CPU to work with */
	Z80 * myZ80 = dynamic_cast<Z80 *>(cpu());

	if(!myZ80) {
		std::cerr << "cpu is not a Z80.\n";
		return;
	}

	m_nmiCycleCounter = 0;
	myZ80->reset();
}


inline void Spectrum::Spectrum::refreshDisplays( void ) {
	for(int i = 0; i < m_displayDevices.count(); ++i) {
		std::cerr << std::time(0) << ": redrawing display.\n";
		SpectrumDisplayDevice * dev = m_displayDevices.item(i);
		if(dev) dev->redrawDisplay(displayMemory());
	}
}


void Spectrum::Spectrum::run( int instructionCount ) {
	Z80 * myZ80 = dynamic_cast<Z80 *>(cpu());

	if(!myZ80) {
		std::cerr << "cpu is not a Z80.\n";
		return;
	}

	int nmiThreshold = myZ80->clockSpeed() / 50;

	/* start main loop */
	while(instructionCount > 0) {
		m_nmiCycleCounter += myZ80->fetchExecuteCycle();

		/* check nmi counter against threshold and raise NMI in CPU if required */
		if(m_nmiCycleCounter > nmiThreshold) {
			myZ80->nmi();
			refreshDisplays();
			m_nmiCycleCounter = 0;

			/* TODO pause based on requested execution speed */
		}

		instructionCount--;
	}
}
