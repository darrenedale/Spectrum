//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_QSPECTRUMKEYBOARD_H
#define SPECTRUM_QSPECTRUMKEYBOARD_H

#include <QObject>
#include <vector>

#include "../keyboard.h"

using SpectrumKeyboard = Spectrum::Keyboard;

namespace Spectrum::Qt
{
    class Keyboard
    : public SpectrumKeyboard,
      public QObject
    {
    protected:
        [[nodiscard]] std::vector<SpectrumKeyboard::Key> mapQtKeys(::Qt::Key) const;

        bool eventFilter(QObject *, QEvent *) override;
    };
}

#endif // SPECTRUM_QSPECTRUMKEYBOARD_H
