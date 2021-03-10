//
// Created by darren on 26/02/2021.
//

#ifndef SCREENVIEW_COLOURCOMBO_H
#define SCREENVIEW_COLOURCOMBO_H

#include <QComboBox>

#include "../displaydevice.h"

namespace ScreenView
{
    class ColourCombo
    : public QComboBox
    {
    Q_OBJECT

    public:
        explicit ColourCombo(QWidget * = nullptr);
        ~ColourCombo() override;

        Spectrum::DisplayDevice::Colour colour() const;
        bool isBright() const;

    public Q_SLOTS:
        void setColour(Spectrum::DisplayDevice::Colour, bool = false);
        void setBright(bool);

    Q_SIGNALS:
        void colourSelected(Spectrum::DisplayDevice::Colour, bool);

    protected:
        int findItem(Spectrum::DisplayDevice::Colour colour, bool = false);
        void addItem(const QString &, Spectrum::DisplayDevice::Colour);
    };
}

#endif //SCREENVIEW_COLOURCOMBO_H
