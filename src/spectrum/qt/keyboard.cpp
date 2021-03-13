//
// Created by darren on 05/03/2021.
//

#include <QEvent>
#include <QKeyEvent>

#include "keyboard.h"

using namespace Spectrum::Qt;

bool Keyboard::eventFilter(QObject * target, QEvent * event)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch (event->type()) {
        case QEvent::Type::KeyPress:
            for (const auto & key : mapQtKeys(static_cast<::Qt::Key>(dynamic_cast<QKeyEvent *>(event)->key()))) {
                setKeyDown(key);
            }
            break;

        case QEvent::Type::KeyRelease:
            for (const auto & key : mapQtKeys(static_cast<::Qt::Key>(dynamic_cast<QKeyEvent *>(event)->key()))) {
                setKeyUp(key);
            }
            break;
    }
#pragma clang diagnostic pop

    return false;
}

std::vector<SpectrumKeyboard::Key> Keyboard::mapQtKeys(::Qt::Key key) const
{
    // TODO configurable mapping
    // TODO mapping single keys to key combinations (e.g. to enter extended mode, graphics mode, ...)
    switch (key) {
        case ::Qt::Key::Key_Backspace:
            return {SpectrumKeyboard::Key::CapsShift, SpectrumKeyboard::Key::Num0};

        case ::Qt::Key::Key_Colon:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::Z};

        case ::Qt::Key::Key_Semicolon:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::O};

        case ::Qt::Key::Key_QuoteDbl:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::P};

        case ::Qt::Key::Key_Apostrophe:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::Num7};

        case ::Qt::Key::Key_Period:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::M};

        case ::Qt::Key::Key_Comma:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::N};

        case ::Qt::Key::Key_Underscore:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::Num0};

        case ::Qt::Key::Key_Minus:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::J};

        case ::Qt::Key::Key_Plus:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::K};

        case ::Qt::Key::Key_Equal:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::L};

        case ::Qt::Key::Key_Greater:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::T};

        case ::Qt::Key::Key_Less:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::R};

        case ::Qt::Key::Key_Slash:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::V};

        case ::Qt::Key::Key_Question:
            return {SpectrumKeyboard::Key::SymbolShift, SpectrumKeyboard::Key::C};

        case ::Qt::Key::Key_CapsLock:
            return {SpectrumKeyboard::Key::CapsShift, SpectrumKeyboard::Key::Num2};

//        case ::Qt::Key::Key_Left:
//            return {SpectrumKeyboard::Key::CapsShift, SpectrumKeyboard::Key::Num5};
//
//        case ::Qt::Key::Key_Right:
//            return {SpectrumKeyboard::Key::CapsShift, SpectrumKeyboard::Key::Num8};
//
//        case ::Qt::Key::Key_Up:
//            return {SpectrumKeyboard::Key::CapsShift, SpectrumKeyboard::Key::Num7};
//
//        case ::Qt::Key::Key_Down:
//            return {SpectrumKeyboard::Key::CapsShift, SpectrumKeyboard::Key::Num6};

        case ::Qt::Key::Key_Shift:
            return {SpectrumKeyboard::Key::CapsShift};

        case ::Qt::Key::Key_Alt:
        case ::Qt::Key::Key_AltGr:
            return {SpectrumKeyboard::Key::SymbolShift};

        case ::Qt::Key::Key_Enter:
        case ::Qt::Key::Key_Return:
            return {SpectrumKeyboard::Key::Enter};

        case ::Qt::Key::Key_Space:
            return {SpectrumKeyboard::Key::Space};

        case ::Qt::Key::Key_1:
            return {SpectrumKeyboard::Key::Num1};

        case ::Qt::Key::Key_2:
            return {SpectrumKeyboard::Key::Num2};

        case ::Qt::Key::Key_3:
            return {SpectrumKeyboard::Key::Num3};

        case ::Qt::Key::Key_4:
            return {SpectrumKeyboard::Key::Num4};

        case ::Qt::Key::Key_5:
            return {SpectrumKeyboard::Key::Num5};

        case ::Qt::Key::Key_6:
            return {SpectrumKeyboard::Key::Num6};

        case ::Qt::Key::Key_7:
            return {SpectrumKeyboard::Key::Num7};

        case ::Qt::Key::Key_8:
            return {SpectrumKeyboard::Key::Num8};

        case ::Qt::Key::Key_9:
            return {SpectrumKeyboard::Key::Num9};

        case ::Qt::Key::Key_0:
            return {SpectrumKeyboard::Key::Num0};

        case ::Qt::Key::Key_Q:
            return {SpectrumKeyboard::Key::Q};

        case ::Qt::Key::Key_W:
            return {SpectrumKeyboard::Key::W};

        case ::Qt::Key::Key_E:
            return {SpectrumKeyboard::Key::E};

        case ::Qt::Key::Key_R:
            return {SpectrumKeyboard::Key::R};

        case ::Qt::Key::Key_T:
            return {SpectrumKeyboard::Key::T};

        case ::Qt::Key::Key_Y:
            return {SpectrumKeyboard::Key::Y};

        case ::Qt::Key::Key_U:
            return {SpectrumKeyboard::Key::U};

        case ::Qt::Key::Key_I:
            return {SpectrumKeyboard::Key::I};

        case ::Qt::Key::Key_O:
            return {SpectrumKeyboard::Key::O};

        case ::Qt::Key::Key_P:
            return {SpectrumKeyboard::Key::P};

        case ::Qt::Key::Key_A:
            return {SpectrumKeyboard::Key::A};

        case ::Qt::Key::Key_S:
            return {SpectrumKeyboard::Key::S};

        case ::Qt::Key::Key_D:
            return {SpectrumKeyboard::Key::D};

        case ::Qt::Key::Key_F:
            return {SpectrumKeyboard::Key::F};

        case ::Qt::Key::Key_G:
            return {SpectrumKeyboard::Key::G};

        case ::Qt::Key::Key_H:
            return {SpectrumKeyboard::Key::H};

        case ::Qt::Key::Key_J:
            return {SpectrumKeyboard::Key::J};

        case ::Qt::Key::Key_K:
            return {SpectrumKeyboard::Key::K};

        case ::Qt::Key::Key_L:
            return {SpectrumKeyboard::Key::L};

        case ::Qt::Key::Key_Z:
            return {SpectrumKeyboard::Key::Z};

        case ::Qt::Key::Key_X:
            return {SpectrumKeyboard::Key::X};

        case ::Qt::Key::Key_C:
            return {SpectrumKeyboard::Key::C};

        case ::Qt::Key::Key_V:
            return {SpectrumKeyboard::Key::V};

        case ::Qt::Key::Key_B:
            return {SpectrumKeyboard::Key::B};

        case ::Qt::Key::Key_N:
            return {SpectrumKeyboard::Key::N};

        case ::Qt::Key::Key_M:
            return {SpectrumKeyboard::Key::M};

        default:
            return {};
    }
}
