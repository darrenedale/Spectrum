//
// Created by darren on 04/04/2021.
//

#ifndef SPECTRUM_MOUSEINTERFACE_H
#define SPECTRUM_MOUSEINTERFACE_H

#include "../z80/iodevice.h"

namespace Spectrum
{
    class MouseInterface
    : public ::Z80::IODevice
    {
    public:
        [[nodiscard]] bool checkWritePort(::Z80::UnsignedWord) const override
        {
            return false;
        }

        void writeByte(::Z80::UnsignedWord, ::Z80::UnsignedByte) override
        {}

        [[nodiscard]] virtual inline int x() const
        {
            return m_x;
        }

        [[nodiscard]] virtual inline int y() const
        {
            return m_y;
        }

        [[nodiscard]] virtual bool button1Pressed() const;
        [[nodiscard]] virtual bool button2Pressed() const;
        [[nodiscard]] virtual bool button3Pressed() const;

        virtual void setX(int x);
        virtual void setY(int y);
        virtual void setButton1Pressed(bool);
        virtual void setButton2Pressed(bool);
        virtual void setButton3Pressed(bool);

    protected:
        int m_x;
        int m_y;
        std::uint8_t m_buttons = 0x0000;
    };
}

#endif //SPECTRUM_MOUSEINTERFACE_H
