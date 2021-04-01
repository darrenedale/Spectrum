//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_BASESPECTRUM_H
#define SPECTRUM_BASESPECTRUM_H

#include <ostream>

#include "../computer.h"
#include "z80.h"

namespace Spectrum
{
    class DisplayDevice;
    class Keyboard;
    class JoystickInterface;

    /**
     * Base class for models of Spectrum.
     */
    class BaseSpectrum
    : public Computer<::Z80::UnsignedByte>
    {
    public:
        using DisplayDevices = std::vector<DisplayDevice *>;

        /**
         * Initialise a new Spectrum borrowing the given memory.
         *
         * The provided memory is borrowed and will not be deleted when te Spectrum is destroyed - it is up to the caller to
         * ensure that it is destroyed at the appropriate time.
         *
         * No ROM is loaded into the 0x0000 - 0x3fff address space. Effectively, the Spectrum will have no code to run
         * unless you specifically provide it with some.
         *
         * @param memory
         */
        explicit BaseSpectrum(MemoryType * memory = nullptr);

        /**
         * Initialise a new Spectrum with a given memory size.
         *
         * The spectrum will allocate its own Memory object and will delete it when the Spectrum is destroyed. The memory
         * size should only be either 32k (Spectrum16K) or 64k (Spectrum48k). The memory created will be a single linear map
         * of the Z80 address space starting at 0x0000.
         *
         * No ROM is loaded into the 0x0000 - 0x3fff address space. Effectively, the Spectrum will have no code to run
         * unless you specifically provide it with some.
         *
         * @param romFile
         * @param memory
         */
        explicit BaseSpectrum(MemoryType::Size memorySize);

        /**
         * Initialise a new Spectrum, loading the ROM from the given file and borrowing the given memory.
         *
         * The ROM file is expected to be 16kb in size, and will be loaded at 0x0000 in the Z80 address space. Note that
         * later models of Spectrum support multiple ROMs that are paged in and out, so constructors for those models should
         * probably use one of the non-ROM-loading base constructors.
         *
         * The provided memory is borrowed and will not be deleted when te Spectrum is destroyed - it is up to the caller to
         * ensure that it is destroyed at the appropriate time.
         *
         * @param romFile
         * @param memory
         */
        explicit BaseSpectrum(const std::string & romFile, MemoryType * memory = nullptr);

        /**
         * Initialise a new Spectrum, loading the ROM from the given file and allocating its own memory.
         *
         * The ROM file is expected to be 16kb in size, and will be loaded at 0x0000 in the Z80 address space. Note that
         * later models of Spectrum support multiple ROMs that are paged in and out, so constructors for those models should
         * probably use one of the non-ROM-loading base constructors.
         *
         * The spectrum will allocate its own Memory object and will delete it when the Spectrum is destroyed. The memory
         * size should only be either 32k (Spectrum16K) or 64k (Spectrum48k). The memory created will be a single linear map
         * of the Z80 address space starting at 0x0000.
         *
         * @param romFile
         * @param memorySize The number of bytes of available memory.
         */
        explicit BaseSpectrum(const std::string & romFile, MemoryType::Size memorySize);

        /**
         * Destroy the Spectrum.
         *
         * The Z80 will be destroyed, and the memory will be destroyed if it is owned by the Spectrum.
         */
        ~BaseSpectrum() override;

        /**
         * Convenience method to fetch the Spectrum's Z80 CPU.
         *
         * @return
         */
        [[nodiscard]] inline Z80 * z80() const
        {
            return dynamic_cast<Z80 *>(cpu());
        }

        /**
         * Fetch the internal counter that manages when the interrupt is generated.
         *
         * This is the current number of t-states that have passed since the last interrupt was generated (or since the
         * Spectrum was last reset if no interrupts have yet been generated).
         *
         * @return
         */
        [[nodiscard]] inline int interruptCounter() const
        {
            return m_interruptTStateCounter;
        }

        /**
         * Fetch the display memory.
         *
         * For all models of Spectrum, regardless of the underlying memory model, this method guarantees that the
         * provided pointer is to a contiguous block of the 6144 bytes in the Spectrum display file. In all cases, the
         * display file is located at a fixed location and exists entirely within a single memory bank.
         *
         * For 128k models, the pointer returned should honour the current video page setting - if the shadow page file
         * is active, it should return a pointer to that memory (in bank 7), otherwise it should return a pointer to the
         * usual display file memory (in bank 2).
         *
         * @return
         */
        [[nodiscard]] virtual ::Z80::UnsignedByte * displayMemory() const = 0;

        /**
         * The size of the display memory.
         *
         * For all models, this is always 6144.
         *
         * @return
         */
        [[nodiscard]] virtual int displayMemorySize() const = 0;

        /**
         * Reset the spectrum.
         *
         * The memory is cleared, the ROM file is re-loaded, the CPU is reset, the interrupt counter is reset and the
         * displays are refreshed.
         */
        void reset() override;

        /**
         * Run a given number of instructions.
         *
         * Instructions are repeatedly fetched from the memory from the location indicated by the program counter and
         * executed until the given number of instructions has been executed. If an interrupt occurs, it is accepted and
         * any instructions that the interrupt handler executes count towards the number of instructions to run.
         *
         * Do not call this method unless you are certain that the Spectrum has a CPU. Upon construction, all Spectrum
         * objects will have a single Z80 CPU, but it is possible for external code to remove it.
         *
         * @param instructionCount
         */
        void run(int instructionCount) override;

        /**
         * Set the execution speed as a percentage of that of a real Spectrum 48k.
         *
         * Note that this does not automatically turn on constrained speed - if speed is not currently constrained
         * you also need to call setExecutionSpeedConstrained().
         *
         * @param percent
         */
        void setExecutionSpeed(int percent)
        {
            m_executionSpeed = (percent / 100.0);
        }

        /**
         * Set the execution speed as a ratio of that of a real Spectrum 48k.
         *
         * Note that this does not automatically turn on constrained speed - if speed is not currently constrained
         * you also need to call setExecutionSpeedConstrained().
         *
         * @param percent
         */
        void setExecutionSpeed(double ratio)
        {
            m_executionSpeed = ratio;
        }

        /**
         * Fetch the current execution speed ratio.
         *
         * Note that this provides the speed ratio that has been set even if the speed is not currently constrained.
         *
         * @return
         */
        [[nodiscard]] double executionSpeed() const
        {
            return m_executionSpeed;
        }

        /**
         * Fetch the current execution speed in %.
         *
         * Note that this provides the speed ratio that has been set even if the speed is not currently constrained.
         *
         * @return
         */
        [[nodiscard]] int executionSpeedPercent() const
        {
            return static_cast<int>(m_executionSpeed * 100);
        }

        /**
         * If set, the Spectrum will be constrained to run no faster than the clock speed set.
         *
         * If not set, it will run as fast as the host CPU will allow.
         *
         * @param constraint
         */
        void setExecutionSpeedConstrained(bool constrain)
        {
            m_constrainExecutionSpeed = constrain;
        }

        /**
         * Check whether the Spectrum is running at approximately the speed determined by the clock speed.
         *
         * @return
         */
        [[nodiscard]] bool executionSpeedConstrained() const
        {
            return m_constrainExecutionSpeed;
        }

        /**
         * Add a display device to the Spectrum.
         *
         * You can add multiple display devices if you wish - they will all output the same screen.
         *
         * Do not provide a null display device. To remove displays, call removeDisplayDevice().
         *
         * The display device is borrowed, not owned. It is the caller's responsibility to ensure it is destroyed at the
         * appropriate time and to ensure that the Spectrum does not retain a reference to a display device that has been
         * destroyed.
         *
         * @param dev
         */
        void addDisplayDevice(DisplayDevice * dev);

        /**
         * Remove a display device from the Spectrum.
         *
         * It is safe to provide a pointer to a display device that is not connected to the Spectrum. Doing so is just a
         * NOOP.
         * @param dev
         */
        void removeDisplayDevice(DisplayDevice * dev);

        /**
         * Fetch an immutable vector of all the display devices attached to the Spectrum.
         *
         * @return
         */
        [[nodiscard]] const DisplayDevices & displayDevices() const
        {
            return m_displayDevices;
        }

        /**
         * Set the keyboard device for the spectrum.
         *
         * The keyboard is borrowed, not owned. It is the caller's responsibility to ensure it is destroyed at the
         * appropriate time and to ensure that the Spectrum does not retain a reference to a keyboard that has been
         * destroyed.
         *
         * It is safe to provide a null keyboard if you just want to remove the existing keyboard from the Spectrum.
         *
         * @param keyboard
         */
        void setKeyboard(Keyboard * keyboard);

        /**
         * Set the joystick interface for the spectrum.
         *
         * The joystick interface is borrowed, not owned. It is the caller's responsibility to ensure it is destroyed at the
         * appropriate time and to ensure that the Spectrum does not retain a reference to a joystick interface that has
         * been destroyed.
         *
         * It is safe to provide a null joystick interface if you just want to remove the existing joystick interface from
         * the Spectrum.
         *
         * @param keyboard
         */
        void setJoystickInterface(JoystickInterface *);

        /**
         * Fetch the current keyboard device for the Spectrum.
         *
         * This can be null if no keyboard has been attached.
         * @return
         */
        [[nodiscard]] Keyboard * keyboard() const
        {
            return m_keyboard;
        }

        /**
         * Fetch the current joystick interface for the Spectrum.
         *
         * This can be null if no joystick interface has been attached.
         * @return
         */
        [[nodiscard]] JoystickInterface * joystickInterface() const
        {
            return m_joystick;
        }

        /**
         * Ask all connected display devices to redraw the Spectrum display.
         */
        inline void refreshDisplays() const;

#if(!defined(NDEBUG))
        void dumpState(std::ostream & = std::cout) const;
#endif

    protected:
        /**
         * Load a file into the ROM address space 0x0000 - 0x3fff.
         *
         * The file is loaded verbatim without verification. If the load fails, the Spectrum will be in an undefined
         * state.
         *
         * @param fileName
         * @return
         */
        bool loadRom(const std::string & fileName);

    private:
        double m_executionSpeed;
        int m_interruptTStateCounter;
        bool m_constrainExecutionSpeed;
        DisplayDevices m_displayDevices;
        Keyboard * m_keyboard;
        JoystickInterface * m_joystick;
        std::string m_romFile;
    };
}

#endif //SPECTRUM_BASESPECTRUM_H
