//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_QKEMPSTONJOYSTICK_H
#define SPECTRUM_QKEMPSTONJOYSTICK_H

#include <QtGamepad/QGamepad>

#include "../kempstonjoystick.h"

namespace Spectrum::Qt
{
    class QKempstonJoystick
    : public QObject,
      public KempstonJoystick
    {
    public:
//        explicit QKempstonJoystick(int deviceId);
//        explicit QKempstonJoystick(QGamepad * gamePad);
//        QKempstonJoystick(const QKempstonJoystick &);
//        QKempstonJoystick(QKempstonJoystick &&) noexcept;
//        QKempstonJoystick & operator=(const QKempstonJoystick &);
//        QKempstonJoystick & operator=(QKempstonJoystick &&) noexcept;
//        ~QKempstonJoystick() override;
//
//        [[nodiscard]] bool checkReadPort(Z80::UnsignedWord port) const override
//        {
//            return m_gamePad && KempstonJoystick::checkReadPort(port);
//        }
//
//        Z80::UnsignedByte readByte(Z80::UnsignedWord port) override;
//
//        [[nodiscard]] bool joystick1IsLeft() const override
//        {
//            return m_gamePad && m_gamePad->buttonLeft();
//        }
//
//        [[nodiscard]] bool joystick1IsRight() const override
//        {
//            return m_gamePad && m_gamePad->buttonRight();
//        }
//
//        [[nodiscard]] bool joystick1IsUp() const override
//        {
//            return m_gamePad && m_gamePad->buttonUp();
//        }
//
//        [[nodiscard]] bool joystick1IsDown() const override
//        {
//            return m_gamePad && m_gamePad->buttonDown();
//        }
//
//        [[nodiscard]] bool joystick1Button1IsPressed() const override
//        {
//            return m_gamePad && m_gamePad->buttonX();
//        }
//
//        [[nodiscard]] bool joystick1Button2IsPressed() const override
//        {
//            return m_gamePad && m_gamePad->buttonA();
//        }
//
//        [[nodiscard]] bool joystick1Button3IsPressed() const override
//        {
//            return m_gamePad && m_gamePad->buttonB();
//        }
//
        bool eventFilter(QObject *, QEvent *) override;
        
    private:
//        bool m_borrowedGamePad;
//        QGamepad * m_gamePad;
    };
}

#endif //SPECTRUM_QKEMPSTONJOYSTICK_H
