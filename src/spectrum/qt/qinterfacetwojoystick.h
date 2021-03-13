//
// Created by darren on 13/03/2021.
//

#ifndef SPECTRUM_QT_QINTERFACETWOJOYSTICK_H
#define SPECTRUM_QT_QINTERFACETWOJOYSTICK_H

#include <QObject>

#include "../interfacetwojoystick.h"

class QEvent;

namespace Spectrum::Qt
{
    /**
     * This is a keymapped "virtual" joystick.
     *
     * TODO rename so that it's clear it's a keymapped joystick so that an actual gamepad-based class can use the
     * QInterfaceTwoJoystick name
     *
     * TODO Consider whether the functionality could be a mixin - QJoystickInterface - that is imported into "shell"
     * classes that inherit from the appropriate base - kempston, IF2, etc. It's currently identical functionality for
     * this class and QKempstonJoystick.
     */
    class QInterfaceTwoJoystick
    : public QObject,
      public InterfaceTwoJoystick
    {
    public:
        explicit QInterfaceTwoJoystick(QObject * parent = nullptr);
        bool eventFilter(QObject *, QEvent *) override;
    };
}

#endif // SPECTRUM_QT_QINTERFACETWOJOYSTICK_H
