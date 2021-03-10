//
// Created by darren on 05/03/2021.
//

#include "qkempstonjoystick.h"

using namespace Spectrum::Qt;

QKempstonJoystick::QKempstonJoystick(QGamepad * gamePad)
: m_gamePad(gamePad),
  m_borrowedGamePad(true)
{
}

QKempstonJoystick::QKempstonJoystick(int deviceId)
: QKempstonJoystick(new QGamepad(deviceId))
{
    m_borrowedGamePad = false;
}

QKempstonJoystick::QKempstonJoystick(const QKempstonJoystick & other)
: KempstonJoystick(other),
  m_gamePad(other.m_gamePad ? new QGamepad(other.m_gamePad->deviceId()) : nullptr),
  m_borrowedGamePad(false)
{
}

QKempstonJoystick::QKempstonJoystick(QKempstonJoystick && other) noexcept
: m_gamePad(other.m_gamePad),
  m_borrowedGamePad(other.m_borrowedGamePad)
{
    other.m_gamePad = nullptr;
}

QKempstonJoystick & QKempstonJoystick::operator=(const QKempstonJoystick & other)
{
    m_gamePad = other.m_gamePad ? new QGamepad(other.m_gamePad->deviceId()) : nullptr;
    m_borrowedGamePad = false;
    return *this;
}

QKempstonJoystick & QKempstonJoystick::operator=(QKempstonJoystick && other) noexcept
{
    m_gamePad = other.m_gamePad;
    m_borrowedGamePad = other.m_borrowedGamePad;
    other.m_gamePad = nullptr;
    return *this;
}

QKempstonJoystick::~QKempstonJoystick()
{
    if (!m_borrowedGamePad) {
        delete m_gamePad;
    }
}

Z80::UnsignedByte QKempstonJoystick::readByte(Z80::UnsignedWord port)
{
    if (!m_gamePad || !QGamepadManager::instance()->isGamepadConnected(m_gamePad->deviceId())) {
        return 0;
    }

    return KempstonJoystick::readByte(port);
}
