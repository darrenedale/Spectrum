//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_BASESPECTRUM_H
#define SPECTRUM_BASESPECTRUM_H

#include <ostream>
#include <memory>
#include "../computer.h"
#include "z80.h"
#include "types.h"

namespace Spectrum
{
    class DisplayDevice;
    class Keyboard;
    class JoystickInterface;
    class MouseInterface;
    class Snapshot;

    /**
     * Abstract base class for models of Spectrum.
     */
    class BaseSpectrum
    : public Computer<::Z80::UnsignedByte>
    {
    public:
        using MemoryType = Memory<::Z80::UnsignedByte>;
        using DisplayDevices = std::vector<DisplayDevice *>;

        /**
         * Destroy the Spectrum.
         *
         * The Z80 will be destroyed, and the memory will be destroyed if it is owned by the Spectrum.
         */
        ~BaseSpectrum() override;

        /**
         * Which model of Spectrum is it?
         *
         * @return
         */
        [[nodiscard]] virtual Model model() const = 0;

        /**
         * Create a snapshot from the Spectrum.
         * @return
         */
        [[nodiscard]] virtual std::unique_ptr<Snapshot> snapshot() const = 0;

        /**
         * Check whether a snapshot can be applied to a Spectrum.
         *
         * @param snapshot The Snapshot to check.
         * @return true if the snapshot can be applied to this Spectrum, false otherwise.
         */
        [[nodiscard]] virtual bool canApplySnapshot(const Snapshot & snapshot) = 0;

        /**
         * Convenience overload to check whether a snapshot pointer can be applied to a Spectrum.
         *
         * Delegates to the const reference overload.
         *
         * @param snapshot The pointer to the snapshot to check. Must not be nullptr.
         * @return true if the snapshot can be applied to this Spectrum, false otherwise.
         */
        [[nodiscard]] virtual bool canApplySnapshot(const Snapshot * snapshot)
        {
            assert(snapshot);
            return canApplySnapshot(*snapshot);
        }

        /**
         * Apply a snapshot to the Spectrum.
         *
         * It is the caller's responsibility to ensure that the snapshot can be applied to the Spectrum model. See
         * canApplySnapshot().
         *
         * @param snapshot The snapshot to apply.
         */
        virtual void applySnapshot(const Snapshot & snapshot) = 0;

        /**
         * Convenience overload to apply a snapshot pointer.
         *
         * Delegates to the const reference overload.
         *
         * @param snapshot Pointer to the snapshot to apply. Must not be nullptr.
         */
        virtual void applySnapshot(const Snapshot * snapshot)
        {
            assert (snapshot);
            applySnapshot(*snapshot);
        }

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
         * Fetch the Spectrum's display file.
         *
         * For 128k models, the display file returned should honour the current video page setting - if the shadow page
         * file is active, it should return a span representing the shadow display file (in bank 7), otherwise it should
         * return a span representing the usual display file in bank 2.
         *
         * @return The current display file.
         */
        [[nodiscard]] virtual DisplayFile displayMemory() const = 0;

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
         * Set the mouse interface for the spectrum.
         *
         * The mouse interface is borrowed, not owned. It is the caller's responsibility to ensure it is destroyed at the
         * appropriate time and to ensure that the Spectrum does not retain a reference to a mouse interface that has
         * been destroyed.
         *
         * It is safe to provide a null mouse interface if you just want to remove the existing mouse interface from
         * the Spectrum.
         *
         * @param mouse
         */
        void setMouseInterface(MouseInterface *);

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
         * Initialise a new Spectrum that owns the given memory.
         *
         * The provided memory is owned and will be deleted when te Spectrum is destroyed.
         *
         * No ROM is loaded into the 0x0000 - 0x3fff address space. Effectively, the Spectrum will have no code to run unless you specifically provide it with
         * some.
         *
         * @param memory The memory for the Spectrum.
         */
        explicit BaseSpectrum(std::unique_ptr<MemoryType> memory = nullptr);

        /**
         * Ask the spectrum to reload its ROM files.
         */
        virtual void reloadRoms() = 0;

        /**
         * Helper to apply the CPU state from a Snapshot to the current Spectrum.
         *
         * This is common to all Spectrum models.
         *
         * @param snapshot The snapshot with the CPU state to apply.
         */
        void applySnapshotCpuState(const Snapshot & snapshot);

    private:
        double m_executionSpeed;
        int m_interruptTStateCounter;
        bool m_constrainExecutionSpeed;
        DisplayDevices m_displayDevices;
        Keyboard * m_keyboard;
        JoystickInterface * m_joystick;
        MouseInterface * m_mouse;
    };
}

#endif //SPECTRUM_BASESPECTRUM_H
