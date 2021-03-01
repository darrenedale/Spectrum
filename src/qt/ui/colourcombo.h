//
// Created by darren on 26/02/2021.
//

#ifndef SCREENVIEW_COLOURCOMBO_H
#define SCREENVIEW_COLOURCOMBO_H

#include <QComboBox>

#include "../../emulator/spectrumdisplaydevice.h"

namespace ScreenView
{
    class ColourCombo
    : public QComboBox
    {
    Q_OBJECT

    public:
        explicit ColourCombo(QWidget * = nullptr);
        ~ColourCombo() override;

        Spectrum::SpectrumDisplayDevice::Colour colour() const;
        bool isBright() const;

    public Q_SLOTS:
        void setColour(Spectrum::SpectrumDisplayDevice::Colour, bool = false);
        void setBright(bool);

    Q_SIGNALS:
        void colourSelected(Spectrum::SpectrumDisplayDevice::Colour, bool);

    protected:
        int findItem(Spectrum::SpectrumDisplayDevice::Colour colour, bool = false);
        void addItem(const QString &, Spectrum::SpectrumDisplayDevice::Colour);
    };
}

#endif //SCREENVIEW_COLOURCOMBO_H
