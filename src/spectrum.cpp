#include "spectrum.h"

#include <cstring>
#include <iostream>
#include <chrono>

#include <QThread>

#include "spectrumdisplaydevice.h"
#include "z80.h"

Spectrum::Spectrum::Spectrum(int memsize, uint8_t * mem)
:	Computer(memsize, mem)
{
    Cpu * z80 = new Z80(memory(), memorySize());
    // 3.5MHz
    z80->setClockSpeed(3500000);
    addCpu(z80);
}

Spectrum::Spectrum::~Spectrum()
{
	// base class takes care of RAM
	Cpu * cpu = z80();
	removeCpu(cpu);
	delete cpu;
}

void Spectrum::Spectrum::reset()
{
    clearMemory();

    if (!loadRom("spectrum48.rom")) {
        return;
    }

	refreshDisplays();

	// fetch the CPU to work with
	Z80 * z80 = dynamic_cast<Z80 *>(cpu());

	if (!z80) {
		std::cerr << "cpu is not a Z80.\n";
		return;
	}

	m_nmiCycleCounter = 0;
    z80->reset();
}

void Spectrum::Spectrum::clearMemory() const
{
    std::memset(m_ram, 0, memorySize());
}

inline void Spectrum::Spectrum::refreshDisplays() const
{
	for (auto * display : m_displayDevices) {
        display->redrawDisplay(displayMemory());
	}
}

void Spectrum::Spectrum::run(int instructionCount)
{
    using namespace std::chrono_literals;
    assert(z80());
	int nmiThreshold = z80()->clockSpeed() / 50;

	while (0 < instructionCount) {
		m_nmiCycleCounter += z80()->fetchExecuteCycle();

		// check nmi counter against threshold and raise NMI in CPU if required
		if (m_nmiCycleCounter > nmiThreshold) {
			z80()->nmi();
			refreshDisplays();
			m_nmiCycleCounter %= nmiThreshold;

			// TODO pause based on requested execution speed
			std::this_thread::sleep_for(50ms);
		}

		instructionCount--;
	}
}

bool Spectrum::Spectrum::loadRom(const std::string &fileName) const
{
    // load ROM into lowest 16K of RAM
    uint8_t * rom = memory();
    std::FILE * romFile = std::fopen("spectrum48.rom", "rb");

    if (!romFile) {
        std::cerr << "spectrum ROM file \"spectrum48.rom\" could not be opened.\n";
        return false;
    }

    std::size_t bytesToRead = 16384;

    while (0 < bytesToRead && !std::feof(romFile)) {
        auto bytesRead = std::fread(rom, sizeof(uint8_t), bytesToRead, romFile);

        if (0 == bytesRead) {
            std::cerr << "failed to load spectrum ROM.\n";
            return false;
        }

        rom += bytesRead;
        bytesToRead -= bytesRead;
    }

    return true;
}
