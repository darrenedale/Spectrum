//
// Created by darren on 04/04/2021.
//

#ifndef SPECTRUM_MOUSEINTERFACE_H
#define SPECTRUM_MOUSEINTERFACE_H

#include "../z80/iodevice.h"

namespace Spectrum
{
    /**
     * Abstract representation of an emulated mouse interface for a Spectrum.
     */
    class MouseInterface
    : public ::Z80::IODevice
    {
    public:
        /**
         * A default implementation returning false - mouse interfaces are generally read-only.
         *
         * @return
         */
        [[nodiscard]] bool checkWritePort(::Z80::UnsignedWord) const override
        {
            return false;
        }

        /**
         * A default empty implementation - mouse interfaces are generally read-only.
         */
        void writeByte(::Z80::UnsignedWord, ::Z80::UnsignedByte) override
        {}

        /**
         * Fetch the current x-coordinate of the mouse pointer.
         *
         * The range of the returned value is defined by the specific interface.
         *
         * @return
         */
        [[nodiscard]] virtual inline int x() const
        {
            return m_x;
        }

        /**
         * Fetch the current y-coordinate of the mouse pointer.
         *
         * The range of the returned value is defined by the specific interface.
         *
         * @return
         */
        [[nodiscard]] virtual inline int y() const
        {
            return m_y;
        }

        /**
         * Check whether the primary (usually left) button is pressed.
         *
         * @return true if it's currently pressed, false if not.
         */
        [[nodiscard]] virtual bool button1Pressed() const;

        /**
         * Check whether the secondary (usually right) button is pressed.
         *
         * @return true if it's currently pressed, false if not or if there is no secondary button.
         */
        [[nodiscard]] virtual bool button2Pressed() const;

        /**
         * Check whether the tertiary (usually middle) button is pressed.
         *
         * @return true if it's currently pressed, false if not or if there is no tertiary button.
         */
        [[nodiscard]] virtual bool button3Pressed() const;

        /**
         * Set the current x-coordinate of the mouse pointer.
         *
         * Implementations of specific interfaces can call this in response to mouse events on the host to emulate the movement of the emulated mouse. Use of
         * this mechanism is not mandatory, it is possible to implement mouse interfaces by reading mouse state in real-time inside readByte(); for event-driven
         * host frameworks, using the provided mechanism is probably easiest.
         *
         * @param x
         */
        virtual void setX(int x);

        /**
         * Set the current y-coordinate of the mouse pointer.
         *
         * Implementations of specific interfaces can call this in response to mouse events on the host to emulate the movement of the emulated mouse. Use of
         * this mechanism is not mandatory, it is possible to implement mouse interfaces by reading mouse state in real-time inside readByte(); for event-driven
         * host frameworks, using the provided mechanism is probably easiest.
         *
         * @param x
         */
        virtual void setY(int y);

        /**
         * Set the current state of the primary button.
         *
         * Implementations of specific interfaces can call this in response to mouse button events on the host to emulate the button state of the emulated
         * mouse. Use of this mechanism is not mandatory, it is possible to implement mouse interfaces by reading mouse state in real-time inside readByte();
         * for event-driven host frameworks, using the provided mechanism is probably easiest.
         *
         * @param true if the button is currently pressed, false if it is currently released.
         */
        virtual void setButton1Pressed(bool);

        /**
         * Set the current state of the secondary button.
         *
         * Implementations of specific interfaces can call this in response to mouse button events on the host to emulate the button state of the emulated
         * mouse. Use of this mechanism is not mandatory, it is possible to implement mouse interfaces by reading mouse state in real-time inside readByte();
         * for event-driven host frameworks, using the provided mechanism is probably easiest.
         *
         * @param true if the button is currently pressed, false if it is currently released.
         */
        virtual void setButton2Pressed(bool);

        /**
         * Set the current state of the tertiary button.
         *
         * Implementations of specific interfaces can call this in response to mouse button events on the host to emulate the button state of the emulated
         * mouse. Use of this mechanism is not mandatory, it is possible to implement mouse interfaces by reading mouse state in real-time inside readByte();
         * for event-driven host frameworks, using the provided mechanism is probably easiest.
         *
         * @param true if the button is currently pressed, false if it is currently released.
         */
        virtual void setButton3Pressed(bool);

    protected:
        /**
         * The current x-coordinate.
         *
         * It is likely this will only be useful if the setX() and x() methods are used to manage the emulated mouse cursor horizontal location.
         */
        int m_x;

        /**
         * The current y-coordinate.
         *
         * It is likely this will only be useful if the setY() and y() methods are used to manage the emulated mouse cursor vertical location.
         */
        int m_y;

        /**
         * The current state of the emulated mouse buttons.
         *
         * It is likely this will only be useful if the setButtonNPressed() and buttonNPressed() methods are used to manage the emulated mouse button state.
         */
        std::uint8_t m_buttons = 0x0000;
    };
}

#endif //SPECTRUM_MOUSEINTERFACE_H
