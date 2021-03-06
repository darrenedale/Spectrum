//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_QSPECTRUMKEYBOARD_H
#define SPECTRUM_QSPECTRUMKEYBOARD_H

#include <QObject>
#include <optional>

#include "../emulator/spectrumkeyboard.h"

namespace Spectrum
{
    class QSpectrumKeyboard
    : public SpectrumKeyboard,
      public QObject
    {
    protected:
        [[nodiscard]] std::optional<SpectrumKeyboard::Key> mapQtKey(Qt::Key) const;

        bool eventFilter(QObject *, QEvent *) override;
    };
}

#endif //SPECTRUM_QSPECTRUMKEYBOARD_H
