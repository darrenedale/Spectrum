//
// Created by darren on 13/03/2021.
//

#include <iostream>
#include <iomanip>
#include <QEvent>
#include <QKeyEvent>

#include "qinterfacetwojoystick.h"

using namespace Spectrum::Qt;

bool QInterfaceTwoJoystick::eventFilter(QObject * target, QEvent * event)
{
    if (auto keyPressed = QEvent::Type::KeyPress == event->type(); keyPressed || QEvent::Type::KeyRelease == event->type()) {
        auto * keyEvent = dynamic_cast<QKeyEvent *>(event);

        if (!keyEvent->isAutoRepeat()) {
            switch (keyEvent->key()) {
                case ::Qt::Key::Key_Up:
                    if (joystick1IsUp() != keyPressed) {
                        setJoystick1Up(keyPressed);
                    }
                    return true;

                case ::Qt::Key::Key_Down:
                    if (joystick1IsDown() != keyPressed) {
                        setJoystick1Down(keyPressed);
                    }
                    return true;

                case ::Qt::Key::Key_Left:
                    if (joystick1IsLeft() != keyPressed) {
                        setJoystick1Left(keyPressed);
                    }
                    return true;

                case ::Qt::Key::Key_Right:
                    if (joystick1IsRight() != keyPressed) {
                        setJoystick1Right(keyPressed);
                    }
                    return true;

                case ::Qt::Key::Key_Control:
                    if (joystick1Button1IsPressed() != keyPressed) {
                        setJoystick1Button1Pressed(keyPressed);
                    }
                    return true;
            }
        }
    }

    return QObject::eventFilter(target, event);
}

QInterfaceTwoJoystick::QInterfaceTwoJoystick(QObject * parent)
: QObject(parent),
  InterfaceTwoJoystick()
{}
