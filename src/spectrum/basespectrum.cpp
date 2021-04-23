#include "basespectrum.h"
#include <fstream>
#include <chrono>
#include <thread>
#include "displaydevice.h"
#include "joystickinterface.h"
#include "mouseinterface.h"
#include "keyboard.h"
#include "z80.h"
#include "snapshot.h"

namespace
{
    // the default Z80 clock speed for a Spectrum
    const int DefaultClockSpeed = 3500000;
}

namespace Spectrum
{
    BaseSpectrum::BaseSpectrum(MemoryType * memory)
    : Computer(memory, true),
      m_executionSpeed(1.0),
      m_interruptTStateCounter(0),
      m_displayDevices(),
      m_constrainExecutionSpeed(true),
      m_keyboard(nullptr),
      m_joystick(nullptr)
    {
        auto * z80 = new Z80(memory);
        z80->setClockSpeed(DefaultClockSpeed);
        addCpu(z80);
        memory->clear();
        z80->reset();
    }

    BaseSpectrum::~BaseSpectrum()
    {
        // NOTE our constructors always ensure base class owns Memory object and therefore destroys it
        auto * cpu = z80();
        removeCpu(cpu);
        delete cpu;
    }

    void BaseSpectrum::reset()
    {
        memory()->clear();
        reloadRoms();
        refreshDisplays();
        auto * cpu = this->z80();
        assert(cpu);
        m_interruptTStateCounter = 0;
        cpu->reset();
    }

#if (!defined(NDEBUG))

    void BaseSpectrum::dumpState(std::ostream & out) const
    {
        z80()->dumpState(out);
    }
#endif

    inline void BaseSpectrum::refreshDisplays() const
    {
        for (auto * display : m_displayDevices) {
            display->redrawDisplay(displayMemory());
        }
    }

    void BaseSpectrum::run(int instructionCount)
    {
        using namespace std::chrono;
        using namespace std::chrono_literals;

        assert(z80());

        static steady_clock::time_point lastInterrupt = steady_clock::now();
        // TODO standard spectrum refresh is not exactly 50FPS it's 50.08 (69888 t-states)
        int interruptThreshold = static_cast<int>(z80()->clockSpeed() / 50);

        while (0 < instructionCount) {
            m_interruptTStateCounter += z80()->fetchExecuteCycle();

            // check interrupt counter against threshold and raise INT in CPU if required
            if (m_interruptTStateCounter > interruptThreshold) {
                z80()->interrupt(0xff);     // NOTE spectrum leaves 0xff on the bus when generating the interrupt
                refreshDisplays();
                m_interruptTStateCounter %= interruptThreshold;

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

    void BaseSpectrum::setKeyboard(Keyboard * keyboard)
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

    void BaseSpectrum::setJoystickInterface(JoystickInterface * joystick)
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

    void BaseSpectrum::setMouseInterface(MouseInterface * mouse)
    {
        auto * cpu = z80();

        if (cpu && m_mouse) {
            cpu->disconnectIODevice(m_mouse);
        }

        m_mouse = mouse;

        if (cpu && m_mouse) {
            cpu->connectIODevice(m_mouse);
        }
    }

    void BaseSpectrum::addDisplayDevice(DisplayDevice * dev)
    {
        assert(dev);
        m_displayDevices.push_back(dev);
        auto * cpu = z80();

        if (cpu) {
            cpu->connectIODevice(dev);
        }
    }

    void BaseSpectrum::removeDisplayDevice(DisplayDevice * dev)
    {
        const auto pos = std::find(m_displayDevices.begin(), m_displayDevices.end(), dev);

        if (pos == m_displayDevices.end()) {
            return;
        }

        m_displayDevices.erase(pos, pos);
    }

    void BaseSpectrum::applySnapshotCpuState(const Snapshot & snapshot)
    {
        auto * cpu = z80();
        assert(cpu);
        cpu->registers() = snapshot.registers();
        cpu->setIff1(snapshot.iff1);
        cpu->setIff2(snapshot.iff2);
        cpu->setInterruptMode(snapshot.im);
    }

}
