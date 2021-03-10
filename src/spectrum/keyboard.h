//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_KEYBOARD_H
#define SPECTRUM_KEYBOARD_H

#include <iostream>
#include <iomanip>

#include "../z80/iodevice.h"

namespace Spectrum
{
    class Keyboard
    : public Z80::IODevice
    {
    public:
        enum class Key: std::uint8_t
        {
            Num1 = 0, Num2, Num3, Num4, Num5,
            Num6, Num7, Num8, Num9, Num0,
            Q, W, E, R, T,
            Y, U, I, O, P,
            A, S, D, F, G,
            H, J, K, L, Enter,
            CapsShift, Z, X, C, V,
            B, N, M, SymbolShift, Space,
        };

        Keyboard();

        /**
         * Check the state of a key.
         *
         * @param key
         * @return
         */
        [[nodiscard]] virtual bool keyState(Key key) const
        {
            return m_state & (static_cast<std::uint64_t>(0x01) << static_cast<std::uint8_t>(key));
        }

        /**
         * Set the state of a key.
         *
         * @param key
         * @param pressed
         */
        void setKeyState(Key key, bool pressed)
        {
            if (pressed) {
                m_state |= (static_cast<uint64_t>(0x01) << static_cast<std::uint8_t>(key));
            } else {
                m_state &= ~(static_cast<uint64_t>(0x01) << static_cast<std::uint8_t>(key));
            }
        }

        /**
         * Set the state of a key to pressed.
         *
         * @param key
         */
        void setKeyDown(Key key)
        {
            setKeyState(key, true);
        }

        /**
         * Set the state of a key to not pressed.
         *
         * @param key
         */
        void setKeyUp(Key key)
        {
            setKeyState(key, false);
        }

        /**
         * Ask the device if it's interested in providing input on a given port.
         *
         * @param port
         * @return
         */
        [[nodiscard]] bool checkReadPort(Z80::UnsignedWord port) const override;

        /**
         * Keyboards are input-only devices.
         */
        [[nodiscard]] bool checkWritePort(Z80::UnsignedWord port) const override
        {
            return false;
        }

        /**
         * Read a byte from the device using the given port.
         *
         * @param port
         * @return
         */
        Z80::UnsignedByte readByte(Z80::UnsignedWord port) override;

        /**
         * Keyboards are input-only devices.
         */
        void writeByte(Z80::UnsignedWord, Z80::UnsignedByte) override
        {}

    private:
        [[nodiscard]] Z80::UnsignedByte readHalfRow(Key) const;
        [[nodiscard]] Z80::UnsignedByte readHalfRowReverse(Key) const;
        std::uint64_t m_state;
    };
}

#endif //SPECTRUM_KEYBOARD_H
