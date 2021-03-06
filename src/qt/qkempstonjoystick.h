//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_QKEMPSTONJOYSTICK_H
#define SPECTRUM_QKEMPSTONJOYSTICK_H

#include <QtGamepad/QGamepad>

#include "../emulator/kempstonjoystick.h"

namespace Spectrum
{
    class QKempstonJoystick
    : KempstonJoystick
    {
    public:
        explicit QKempstonJoystick(int deviceId);
        explicit QKempstonJoystick(QGamepad * gamePad);
        QKempstonJoystick(const QKempstonJoystick &);
        QKempstonJoystick(QKempstonJoystick &&) noexcept;
        QKempstonJoystick & operator=(const QKempstonJoystick &);
        QKempstonJoystick & operator=(QKempstonJoystick &&) noexcept;
        ~QKempstonJoystick() override;

        [[nodiscard]] bool checkReadPort(Z80::UnsignedWord port) const override
        {
            return m_gamePad && KempstonJoystick::checkReadPort(port);
        }

        Z80::UnsignedByte readByte(Z80::UnsignedWord port) override;

        [[nodiscard]] bool joystickIsLeft() const override
        {
            return m_gamePad && m_gamePad->buttonLeft();
        }
        
        [[nodiscard]] bool joystickIsRight() const override
        {
            return m_gamePad && m_gamePad->buttonRight();
        }
        
        [[nodiscard]] bool joystickIsUp() const override
        {
            return m_gamePad && m_gamePad->buttonUp();
        }
        
        [[nodiscard]] bool joystickIsDown() const override
        {
            return m_gamePad && m_gamePad->buttonDown();
        }
        
        [[nodiscard]] bool button1IsPressed() const override
        {
            return m_gamePad && m_gamePad->buttonX();
        }
        
        [[nodiscard]] bool button2IsPressed() const override
        {
            return m_gamePad && m_gamePad->buttonA();
        }

        [[nodiscard]] bool button3IsPressed() const override
        {
            return m_gamePad && m_gamePad->buttonB();
        }
        
    private:
        bool m_borrowedGamePad;
        QGamepad * m_gamePad;
    };
}

#endif //SPECTRUM_QKEMPSTONJOYSTICK_H
