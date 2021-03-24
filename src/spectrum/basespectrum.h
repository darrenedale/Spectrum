//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_BASESPECTRUM_H
#define SPECTRUM_BASESPECTRUM_H

#include "../computer.h"
#include "z80.h"

namespace Spectrum
{
    class DisplayDevice;
    class Keyboard;
    class JoystickInterface;

    class BaseSpectrum
    : public Computer<>
    {
    public:
        using DisplayDevices = std::vector<DisplayDevice *>;

        explicit BaseSpectrum(int memorySize, uint8_t * memory = nullptr);
        BaseSpectrum(const std::string & romFile, int memorySize, uint8_t * memory = nullptr);
        ~BaseSpectrum() override;

        inline Z80 * z80() const
        {
            return dynamic_cast<Z80 *>(cpu());
        }

        inline int nmiCounter() const
        {
            return m_interruptCycleCounter;
        }

        [[nodiscard]] virtual std::uint8_t * displayMemory() const = 0;
        [[nodiscard]] virtual int displayMemorySize() const = 0;

        void reset() override;
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

        void addDisplayDevice(DisplayDevice * dev);
        void removeDisplayDevice(DisplayDevice * dev);

        [[nodiscard]] const DisplayDevices & displayDevices() const
        {
            return m_displayDevices;
        }

        void setKeyboard(Keyboard * keyboard);
        void setJoystickInterface(JoystickInterface *);

        [[nodiscard]] Keyboard * keyboard() const
        {
            return m_keyboard;
        }

        [[nodiscard]] JoystickInterface * joystickInterface() const
        {
            return m_joystick;
        }

        inline void refreshDisplays() const;

#if(!defined(NDEBUG))
        void dumpState() const;
#endif

    protected:
        void clearMemory() const;
        bool loadRom(const std::string & fileName);

    private:
        double m_executionSpeed;
        int m_interruptCycleCounter;
        bool m_constrainExecutionSpeed;
        DisplayDevices m_displayDevices;
        Keyboard * m_keyboard;
        JoystickInterface * m_joystick;
        std::string m_romFile;
    };
}

#endif //SPECTRUM_BASESPECTRUM_H
