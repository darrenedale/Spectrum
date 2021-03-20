#include "spectrum.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include "displaydevice.h"
#include "joystickinterface.h"
#include "keyboard.h"
#include "z80.h"

namespace
{
    const int DefaultClockSpeed = 3500000;    
}

namespace Spectrum
{
    Spectrum::Spectrum(const std::string & romFile, int memSize, uint8_t * mem)
    : Spectrum(memSize, mem)
    {
        (void) loadRom(romFile);
    }

    Spectrum::Spectrum(int memSize, uint8_t * mem)
    : Computer(memSize, mem),
      m_executionSpeed(1.0),
      m_interruptCycleCounter(0),
      m_displayDevices(),
      m_constrainExecutionSpeed(true),
      m_keyboard(nullptr),
      m_joystick(nullptr)
    {
        auto * z80 = new Z80(memory(), memorySize());
        z80->setClockSpeed(DefaultClockSpeed);
        addCpu(z80);
        clearMemory();
        z80->reset();
    }

    Spectrum::~Spectrum()
    {
        // base class takes care of RAM
        auto * cpu = z80();
        removeCpu(cpu);
        delete cpu;
    }

    void Spectrum::reset()
    {
        clearMemory();

        // TODO load the ROM file
        if (!loadRom(m_romFile)) {
            return;
        }

        refreshDisplays();

        // fetch the CPU to work with
        auto * z80 = this->z80();

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
        int interruptThreshold = static_cast<int>(z80()->clockSpeed() / 50);

        while (0 < instructionCount) {
            m_interruptCycleCounter += z80()->fetchExecuteCycle();

            // check interrupt counter against threshold and raise NMI in CPU if required
            if (m_interruptCycleCounter > interruptThreshold) {
                z80()->interrupt();
                refreshDisplays();
                m_interruptCycleCounter %= interruptThreshold;

                if (m_constrainExecutionSpeed) {
                    // pause based on requested execution speed
                    auto actualInterruptInterval = steady_clock::now() - lastInterrupt;

                    if (actualInterruptInterval < 20ms) {
                        duration sleepFor = (20ms - actualInterruptInterval) / m_executionSpeed;
                        std::this_thread::sleep_for(sleepFor);
                    }

                    lastInterrupt = steady_clock::now();
                }
            }

            instructionCount--;
        }
    }

    bool Spectrum::loadRom(const std::string & fileName)
    {
        static constexpr const std::size_t RomFileSize = 0x4000;
        std::ifstream inFile(fileName);

        if (!inFile) {
            std::cerr << "spectrum ROM file \"" << fileName << "\" could not be opened.\n";
            return false;
        }

        inFile.read(reinterpret_cast<std::ifstream::char_type *>(memory()), RomFileSize);

        if (inFile.fail()) {
            std::cerr << "failed to load spectrum ROM file \"" << fileName << "\".\n";
            return false;
        }

        m_romFile = fileName;
        return true;
    }

    void Spectrum::setKeyboard(Keyboard * keyboard)
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

    void Spectrum::setJoystickInterface(JoystickInterface * joystick)
    {
        auto * cpu = z80();

        if (cpu && m_joystick) {
            cpu->disconnectIODevice(m_joystick);
        }

        m_joystick = joystick;

        if (cpu && m_joystick) {
            cpu->connectIODevice(m_joystick);
        }
    }

    void Spectrum::addDisplayDevice(DisplayDevice * dev)
    {
        assert(dev);
        m_displayDevices.push_back(dev);
        auto * cpu = z80();

        if (cpu) {
            cpu->connectIODevice(dev);
        }
    }

    void Spectrum::removeDisplayDevice(DisplayDevice * dev)
    {
        const auto pos = std::find(m_displayDevices.begin(), m_displayDevices.end(), dev);

        if (pos == m_displayDevices.end()) {
            return;
        }

        m_displayDevices.erase(pos, pos);
    }
}
