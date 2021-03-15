//
// Created by darren on 26/02/2021.
//

#include "colourcombo.h"

namespace {
    constexpr const int ColourRole = Qt::UserRole;
    constexpr const int BrightRole = ColourRole + 1;
}

using namespace ScreenView;
using namespace Spectrum;

ColourCombo::ColourCombo(QWidget * parent)
: QComboBox(parent)
{
    addItem(tr("Black"), Colour::Black);
    addItem(tr("Blue"), Colour::Blue);
    addItem(tr("Red"), Colour::Red);
    addItem(tr("Magenta"), Colour::Magenta);
    addItem(tr("Green"), Colour::Green);
    addItem(tr("Cyan"), Colour::Cyan);
    addItem(tr("Yellow"), Colour::Yellow);
    addItem(tr("White"), Colour::White);

    connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this]() {
        Q_EMIT colourSelected(colour(), isBright());
    });
}

ColourCombo::~ColourCombo() = default;

Colour ColourCombo::colour() const
{
    return static_cast<Colour>(currentData(ColourRole).toUInt());
}

bool ColourCombo::isBright() const
{
    return currentData(BrightRole).toBool();
}

void ColourCombo::addItem(const QString & text, Spectrum::Colour colour)
{
    int idx = count();
    QComboBox::addItem(text);
    setItemData(idx, static_cast<std::uint8_t>(colour), ColourRole);
    setItemData(idx, false, BrightRole);

    ++idx;
    QComboBox::addItem(text + tr(" (bright)"));
    setItemData(idx, static_cast<std::uint8_t>(colour), ColourRole);
    setItemData(idx, true, BrightRole);
}

void ColourCombo::setBright(bool bright)
{
    if (isBright() == bright) {
        return;
    }

    int idx = findItem(colour(), bright);

    if (-1 == idx) {
        return;
    }

    setCurrentIndex(idx);
}

void ColourCombo::setColour(Colour colour, bool bright)
{
    if (this->colour() == colour && isBright() == bright) {
        return;
    }

    int idx = findItem(colour, bright);

    if (-1 == idx) {
        return;
    }

    setCurrentIndex(idx);
}

int ColourCombo::findItem(Spectrum::Colour colour, bool bright)
{
    for (int idx = 0; idx < count(); ++idx) {
        if (static_cast<Colour>(itemData(idx, ColourRole).toUInt()) == colour
        && itemData(idx, BrightRole).toBool() == bright) {
            return idx;
        }
    }

    return -1;
}
