//
// Created by darren on 05/03/2021.
//

#include <iostream>
#include <iomanip>
#include <QEvent>
#include <QKeyEvent>

#include "qkempstonjoystick.h"

using namespace Spectrum::Qt;
//
//QKempstonJoystick::QKempstonJoystick(QGamepad * gamePad)
//: m_gamePad(gamePad),
//  m_borrowedGamePad(true)
//{
//}
//
//QKempstonJoystick::QKempstonJoystick(int deviceId)
//: QKempstonJoystick(new QGamepad(deviceId))
//{
//    m_borrowedGamePad = false;
//}
//
//QKempstonJoystick::QKempstonJoystick(const QKempstonJoystick & other)
//: KempstonJoystick(other),
//  m_gamePad(other.m_gamePad ? new QGamepad(other.m_gamePad->deviceId()) : nullptr),
//  m_borrowedGamePad(false)
//{
//}
//
//QKempstonJoystick::QKempstonJoystick(QKempstonJoystick && other) noexcept
//: m_gamePad(other.m_gamePad),
//  m_borrowedGamePad(other.m_borrowedGamePad)
//{
//    other.m_gamePad = nullptr;
//}
//
//QKempstonJoystick & QKempstonJoystick::operator=(const QKempstonJoystick & other)
//{
//    m_gamePad = other.m_gamePad ? new QGamepad(other.m_gamePad->deviceId()) : nullptr;
//    m_borrowedGamePad = false;
//    return *this;
//}
//
//QKempstonJoystick & QKempstonJoystick::operator=(QKempstonJoystick && other) noexcept
//{
//    m_gamePad = other.m_gamePad;
//    m_borrowedGamePad = other.m_borrowedGamePad;
//    other.m_gamePad = nullptr;
//    return *this;
//}
//
//QKempstonJoystick::~QKempstonJoystick()
//{
//    if (!m_borrowedGamePad) {
//        delete m_gamePad;
//    }
//}

//Z80::UnsignedByte QKempstonJoystick::readByte(Z80::UnsignedWord port)
//{
//    if (!m_gamePad || !QGamepadManager::instance()->isGamepadConnected(m_gamePad->deviceId())) {
//        return 0x00;
//    }
//
//    return KempstonJoystick::readByte(port);
//}

bool QKempstonJoystick::eventFilter(QObject * target, QEvent * event)
{
    if (auto keyPressed = QEvent::Type::KeyPress == event->type(); keyPressed || QEvent::Type::KeyRelease == event->type()) {
        auto * keyEvent = dynamic_cast<QKeyEvent *>(event);

        if (!keyEvent->isAutoRepeat()) {
            switch (keyEvent->key()) {
                case ::Qt::Key::Key_Up:
                    if (joystick1IsUp() != keyPressed) {
                        setJoystick1Up(keyPressed);
                        std::cout << "Kempston joystick state: 0x" << std::hex << std::setfill('0') << std::setw(4)
                                  << m_state
                                  << std::dec << std::setfill(' ') << '\n';
                    }
                    return true;

                case ::Qt::Key::Key_Down:
                    if (joystick1IsDown() != keyPressed) {
                        setJoystick1Down(keyPressed);
                        std::cout << "Kempston joystick state: 0x" << std::hex << std::setfill('0') << std::setw(4)
                                  << m_state
                                  << std::dec << std::setfill(' ') << '\n';
                    }
                    return true;

                case ::Qt::Key::Key_Left:
                    if (joystick1IsLeft() != keyPressed) {
                        setJoystick1Left(keyPressed);
                        std::cout << "Kempston joystick state: 0x" << std::hex << std::setfill('0') << std::setw(4)
                                  << m_state
                                  << std::dec << std::setfill(' ') << '\n';
                    }
                    return true;

                case ::Qt::Key::Key_Right:
                    if (joystick1IsRight() != keyPressed) {
                        setJoystick1Right(keyPressed);
                        std::cout << "Kempston joystick state: 0x" << std::hex << std::setfill('0') << std::setw(4)
                                  << m_state
                                  << std::dec << std::setfill(' ') << '\n';
                    }
                    return true;

                case ::Qt::Key::Key_Control:
                    if (joystick1Button1IsPressed() != keyPressed) {
                        setJoystick1Button1Pressed(keyPressed);
                        std::cout << "Kempston joystick state: 0x" << std::hex << std::setfill('0') << std::setw(4)
                                  << m_state
                                  << std::dec << std::setfill(' ') << '\n';
                    }
                    return true;
            }
        }
    }

    return QObject::eventFilter(target, event);
}
