//
// Created by darren on 04/04/2021.
//

#include "mouseinterface.h"
#include "../util/debug.h"

using namespace Spectrum;

namespace
{
    constexpr const int Button1Bit = 0;
    constexpr const int Button2Bit = 1;
    constexpr const int Button3Bit = 2;
}

void MouseInterface::setX(int x)
{
#if (!defined(NDEBUG))
    Util::debug << "setting mouse X to " << x << '\n';
#endif
    m_x = x;
}

void MouseInterface::setY(int y)
{
#if (!defined(NDEBUG))
    Util::debug << "setting mouse Y to " << y << '\n';
#endif
    m_y = y;
}

bool MouseInterface::button1Pressed() const
{
    return m_buttons & (1 << Button1Bit);
}

bool MouseInterface::button2Pressed() const
{
    return m_buttons & (1 << Button2Bit);
}

bool MouseInterface::button3Pressed() const
{
    return m_buttons & (1 << Button3Bit);
}

void MouseInterface::setButton1Pressed(bool pressed)
{
    if (pressed) {
        m_buttons |= (1 << Button1Bit);
    } else {
        m_buttons &= ~(1 << Button1Bit);
    }
}

void MouseInterface::setButton2Pressed(bool pressed)
{
    if (pressed) {
        m_buttons |= (1 << Button2Bit);
    } else {
        m_buttons &= ~(1 << Button2Bit);
    }
}

void MouseInterface::setButton3Pressed(bool pressed)
{
    if (pressed) {
        m_buttons |= (1 << Button3Bit);
    } else {
        m_buttons &= ~(1 << Button3Bit);
    }
}
