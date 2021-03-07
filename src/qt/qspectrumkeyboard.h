//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_QSPECTRUMKEYBOARD_H
#define SPECTRUM_QSPECTRUMKEYBOARD_H

#include <QObject>
#include <vector>

#include "../emulator/spectrumkeyboard.h"

namespace Spectrum
{
    class QSpectrumKeyboard
    : public SpectrumKeyboard,
      public QObject
    {
    protected:
        [[nodiscard]] std::vector<SpectrumKeyboard::Key> mapQtKey(Qt::Key) const;

        bool eventFilter(QObject *, QEvent *) override;
    };
}

#endif //SPECTRUM_QSPECTRUMKEYBOARD_H
