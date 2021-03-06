#include "spectrum.h"

#include <cstring>
#include <iostream>
#include <chrono>

#include <QThread>

#include "spectrumdisplaydevice.h"
#include "spectrumkeyboard.h"
#include "../z80/z80.h"

namespace
{
    const int DefaultClockSpeed = 3500000;    
}

namespace Spectrum
{
    Spectrum::Spectrum(int memsize, uint8_t * mem)
            : Computer(memsize, mem)
    {
        Cpu * z80 = new Z80::Z80(memory(), memorySize());
        // 3.5MHz
        z80->setClockSpeed(DefaultClockSpeed);
        addCpu(z80);
    }

    Spectrum::~Spectrum()
    {
        // base class takes care of RAM
        Cpu * cpu = z80();
        removeCpu(cpu);
        delete cpu;
    }

    void Spectrum::reset()
    {
        clearMemory();

        if (!loadRom("spectrum48.rom")) {
            return;
        }

        refreshDisplays();

        // fetch the CPU to work with
        auto * z80 = dynamic_cast<Z80::Z80 *>(cpu());

        if (!z80) {
            std::cerr << "cpu is not a Z80.\n";
            return;
        }

        m_interruptCycleCounter = 0;
        z80->reset();
    }

    void Spectrum::clearMemory() const
    {
        std::memset(m_ram, 0, memorySize());
    }

    inline void Spectrum::refreshDisplays() const
    {
        for (auto * display : m_displayDevices) {
            display->redrawDisplay(displayMemory());
        }
    }

    void Spectrum::run(int instructionCount)
    {
        using namespace std::chrono;
        using namespace std::chrono_literals;

        assert(z80());

        static steady_clock::time_point lastInterrupt = steady_clock::now();
        // TODO spectrum refresh is not exactly 50FPS
        int interruptThreshold = z80()->clockSpeed() / 50;

        while (0 < instructionCount) {
            m_interruptCycleCounter += z80()->fetchExecuteCycle();

            // check nmi counter against threshold and raise NMI in CPU if required
            if (m_interruptCycleCounter > interruptThreshold) {
                z80()->interrupt();
                refreshDisplays();
                m_interruptCycleCounter %= interruptThreshold;

                if (m_constrainExecutionSpeed) {
                    // pause based on requested execution speed
                    auto actualInterruptInterval = steady_clock::now() - lastInterrupt;

                    if (actualInterruptInterval < 20ms) {
                        auto sleepFor = 20ms - actualInterruptInterval;
                        std::this_thread::sleep_for(sleepFor);
                    }

                    lastInterrupt = steady_clock::now();
                }
            }

            instructionCount--;
        }
    }

    bool Spectrum::loadRom(const std::string & fileName) const
    {
        // load ROM into lowest 16K of RAM
        uint8_t * rom = memory();
        std::FILE * romFile = std::fopen(fileName.c_str(), "rb");

        if (!romFile) {
            std::cerr << "spectrum ROM file \"" << fileName << "\" could not be opened.\n";
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

    void Spectrum::setKeyboard(SpectrumKeyboard * keyboard)
    {
        auto * cpu = z80();

        if (cpu && m_keyboard) {
            cpu->disconnectIODevice(m_keyboard);
        }

        m_keyboard = keyboard;

        if (cpu && keyboard) {
            cpu->connectIODevice(m_keyboard);
        }
    }

    void Spectrum::addDisplayDevice(SpectrumDisplayDevice * dev)
    {
        assert(dev);
        m_displayDevices.push_back(dev);
        auto * cpu = z80();

        if (cpu) {
            cpu->connectIODevice(dev);
        }
    }
}
