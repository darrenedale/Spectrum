//
// Created by darren on 26/02/2021.
//

#ifndef SCREENVIEW_COLOURCOMBO_H
#define SCREENVIEW_COLOURCOMBO_H

#include <QComboBox>

#include "../types.h"

namespace ScreenView
{
    class ColourCombo
    : public QComboBox
    {
    Q_OBJECT

    public:
        explicit ColourCombo(QWidget * = nullptr);
        ~ColourCombo() override;

        [[nodiscard]] Spectrum::Colour colour() const;
        [[nodiscard]] bool isBright() const;

    public Q_SLOTS:
        void setColour(Spectrum::Colour, bool = false);
        void setBright(bool);

    Q_SIGNALS:
        void colourSelected(Spectrum::Colour, bool);

    protected:
        int findItem(Spectrum::Colour colour, bool = false);
        void addItem(const QString &, Spectrum::Colour);
    };
}

#endif //SCREENVIEW_COLOURCOMBO_H
