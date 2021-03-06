//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_KEMPSTONJOYSTICK_H
#define SPECTRUM_KEMPSTONJOYSTICK_H

#include "../z80/z80iodevice.h"

namespace Spectrum
{
    class KempstonJoystick
    : public Z80::Z80IODevice
    {
    public:
        [[nodiscard]] bool checkReadPort(Z80::UnsignedWord port) const override
        {
            return 31 == (port & 0x00ff);
        }

        [[nodiscard]] bool checkWritePort(Z80::UnsignedWord) const override
        {
            return false;
        }

        Z80::UnsignedByte readByte(Z80::UnsignedWord port) override;

        void writeByte(Z80::UnsignedWord, Z80::UnsignedByte) override
        {}

//    protected:
        virtual bool joystickIsLeft() const = 0;
        virtual bool joystickIsRight() const = 0;
        virtual bool joystickIsUp() const = 0;
        virtual bool joystickIsDown() const = 0;
        virtual bool button1IsPressed() const = 0;
        virtual bool button2IsPressed() const = 0;
        virtual bool button3IsPressed() const = 0;
    };
}

#endif //SPECTRUM_KEMPSTONJOYSTICK_H
