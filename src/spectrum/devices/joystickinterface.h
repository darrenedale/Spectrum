//
// Created by darren on 13/03/2021.
//

#ifndef SPECTRUM_JOYSTICKINTERFACE_H
#define SPECTRUM_JOYSTICKINTERFACE_H

#include "../../z80/iodevice.h"

namespace Spectrum::Devices
{
    /**
     * TODO consider separating this into the interface and a mixin (the state and implementations of the accessors and
     *  mutators) that provides a convenient way to implement joysticks (e.g. using Qt events, SDL events, etc.)
     */
    class JoystickInterface
    : public ::Z80::IODevice
    {
    public:
        static constexpr int supportedJoysticks()
        {
            return 0;
        }

        [[nodiscard]]
        virtual int availableJoysticks() const = 0;

        [[nodiscard]] bool checkWritePort(::Z80::UnsignedWord) const override
        {
            return false;
        }

        void writeByte(::Z80::UnsignedWord, ::Z80::UnsignedByte) override
        {}

        [[nodiscard]]
        virtual bool joystick1IsLeft() const;

        [[nodiscard]]
        virtual bool joystick1IsRight() const;

        [[nodiscard]]
        virtual bool joystick1IsUp() const;

        [[nodiscard]]
        virtual bool joystick1IsDown() const;

        [[nodiscard]]
        virtual bool joystick1Button1IsPressed() const;

        [[nodiscard]]
        virtual bool joystick1Button2IsPressed() const;

        [[nodiscard]]
        virtual bool joystick1Button3IsPressed() const;


        [[nodiscard]]
        virtual bool joystick2IsLeft() const;

        [[nodiscard]]
        virtual bool joystick2IsRight() const;

        [[nodiscard]]
        virtual bool joystick2IsUp() const;

        [[nodiscard]]
        virtual bool joystick2IsDown() const;

        [[nodiscard]]
        virtual bool joystick2Button1IsPressed() const;

        [[nodiscard]]
        virtual bool joystick2Button2IsPressed() const;

        [[nodiscard]]
        virtual bool joystick2Button3IsPressed() const;

        void setJoystick1Up(bool up);
        void setJoystick1Down(bool down);
        void setJoystick1Left(bool left);
        void setJoystick1Right(bool right);
        void setJoystick1Button1Pressed(bool pressed);
        void setJoystick1Button2Pressed(bool pressed);
        void setJoystick1Button3Pressed(bool pressed);
        
        void setJoystick2Up(bool up);
        void setJoystick2Down(bool down);
        void setJoystick2Left(bool left);
        void setJoystick2Right(bool right);
        void setJoystick2Button1Pressed(bool pressed);
        void setJoystick2Button2Pressed(bool pressed);
        void setJoystick2Button3Pressed(bool pressed);
        
    protected:
        std::uint16_t m_state = 0x00;
    };
}

#endif //SPECTRUM_JOYSTICKINTERFACE_H
