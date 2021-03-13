//
// Created by darren on 13/03/2021.
//

#include "joystickinterface.h"

using namespace Spectrum;

namespace
{
    constexpr const std::uint16_t Joystick1UpMask = 0x0001;
    constexpr const std::uint16_t Joystick1DownMask = 0x0002;
    constexpr const std::uint16_t Joystick1LeftMask = 0x0004;
    constexpr const std::uint16_t Joystick1RightMask = 0x0008;
    constexpr const std::uint16_t Joystick1Button1Mask = 0x0010;
    constexpr const std::uint16_t Joystick1Button2Mask = 0x0020;
    constexpr const std::uint16_t Joystick1Button3Mask = 0x0040;

    constexpr const std::uint16_t Joystick2UpMask = 0x0100;
    constexpr const std::uint16_t Joystick2DownMask = 0x0200;
    constexpr const std::uint16_t Joystick2LeftMask = 0x0400;
    constexpr const std::uint16_t Joystick2RightMask = 0x0800;
    constexpr const std::uint16_t Joystick2Button1Mask = 0x1000;
    constexpr const std::uint16_t Joystick2Button2Mask = 0x2000;
    constexpr const std::uint16_t Joystick2Button3Mask = 0x4000;
}

bool JoystickInterface::joystick1IsLeft() const
{
    return m_state & Joystick1LeftMask;
}

bool JoystickInterface::joystick1IsRight() const
{
    return m_state & Joystick1RightMask;
}

bool JoystickInterface::joystick1IsUp() const
{
    return m_state & Joystick1UpMask;
}

bool JoystickInterface::joystick1IsDown() const
{
    return m_state & Joystick1DownMask;
}

bool JoystickInterface::joystick1Button1IsPressed() const
{
    return m_state & Joystick1Button1Mask;
}

bool JoystickInterface::joystick1Button2IsPressed() const
{
    return m_state & Joystick1Button2Mask;
}

bool JoystickInterface::joystick1Button3IsPressed() const
{
    return m_state & Joystick1Button3Mask;
}

bool JoystickInterface::joystick2IsLeft() const
{
    return m_state & Joystick2LeftMask;
}

bool JoystickInterface::joystick2IsRight() const
{
    return m_state & Joystick2RightMask;
}

bool JoystickInterface::joystick2IsUp() const
{
    return m_state & Joystick2UpMask;
}

bool JoystickInterface::joystick2IsDown() const
{
    return m_state & Joystick2DownMask;
}

bool JoystickInterface::joystick2Button1IsPressed() const
{
    return m_state & Joystick2Button1Mask;
}

bool JoystickInterface::joystick2Button2IsPressed() const
{
    return m_state & Joystick2Button2Mask;
}

bool JoystickInterface::joystick2Button3IsPressed() const
{
    return m_state & Joystick2Button3Mask;
}

void JoystickInterface::setJoystick1Up(bool up)
{
    if (up) {
        m_state |= Joystick1UpMask;
    } else {
        m_state &= ~Joystick1UpMask;
    }
}

void JoystickInterface::setJoystick1Down(bool down)
{
    if (down) {
        m_state |= Joystick1DownMask;
    } else {
        m_state &= ~Joystick1DownMask;
    }
}

void JoystickInterface::setJoystick1Left(bool left)
{
    if (left) {
        m_state |= Joystick1LeftMask;
    } else {
        m_state &= ~Joystick1LeftMask;
    }
}

void JoystickInterface::setJoystick1Right(bool right)
{
    if (right) {
        m_state |= Joystick1RightMask;
    } else {
        m_state &= ~Joystick1RightMask;
    }
}

void JoystickInterface::setJoystick1Button1Pressed(bool pressed)
{
    if (pressed) {
        m_state |= Joystick1Button1Mask;
    } else {
        m_state &= ~Joystick1Button1Mask;
    }
}

void JoystickInterface::setJoystick1Button2Pressed(bool pressed)
{
    if (pressed) {
        m_state |= Joystick1Button2Mask;
    } else {
        m_state &= ~Joystick1Button2Mask;
    }
}

void JoystickInterface::setJoystick1Button3Pressed(bool pressed)
{
    if (pressed) {
        m_state |= Joystick1Button3Mask;
    } else {
        m_state &= ~Joystick1Button3Mask;
    }
}

void JoystickInterface::setJoystick2Up(bool up)
{
    if (up) {
        m_state |= Joystick2UpMask;
    } else {
        m_state &= ~Joystick2UpMask;
    }
}

void JoystickInterface::setJoystick2Down(bool down)
{
    if (down) {
        m_state |= Joystick2DownMask;
    } else {
        m_state &= ~Joystick2DownMask;
    }
}

void JoystickInterface::setJoystick2Left(bool left)
{
    if (left) {
        m_state |= Joystick2LeftMask;
    } else {
        m_state &= ~Joystick2LeftMask;
    }
}

void JoystickInterface::setJoystick2Right(bool right)
{
    if (right) {
        m_state |= Joystick2RightMask;
    } else {
        m_state &= ~Joystick2RightMask;
    }
}

void JoystickInterface::setJoystick2Button1Pressed(bool pressed)
{
    if (pressed) {
        m_state |= Joystick2Button1Mask;
    } else {
        m_state &= ~Joystick2Button1Mask;
    }
}

void JoystickInterface::setJoystick2Button2Pressed(bool pressed)
{
    if (pressed) {
        m_state |= Joystick2Button2Mask;
    } else {
        m_state &= ~Joystick2Button2Mask;
    }
}

void JoystickInterface::setJoystick2Button3Pressed(bool pressed)
{
    if (pressed) {
        m_state |= Joystick2Button3Mask;
    } else {
        m_state &= ~Joystick2Button3Mask;
    }
}
