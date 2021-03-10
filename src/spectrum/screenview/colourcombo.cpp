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
    addItem(tr("Black"), DisplayDevice::Colour::Black);
    addItem(tr("Blue"), DisplayDevice::Colour::Blue);
    addItem(tr("Red"), DisplayDevice::Colour::Red);
    addItem(tr("Magenta"), DisplayDevice::Colour::Magenta);
    addItem(tr("Green"), DisplayDevice::Colour::Green);
    addItem(tr("Cyan"), DisplayDevice::Colour::Cyan);
    addItem(tr("Yellow"), DisplayDevice::Colour::Yellow);
    addItem(tr("White"), DisplayDevice::Colour::White);

    connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this]() {
        Q_EMIT colourSelected(colour(), isBright());
    });
}

ColourCombo::~ColourCombo() = default;

DisplayDevice::Colour ColourCombo::colour() const
{
    return static_cast<DisplayDevice::Colour>(currentData(ColourRole).toUInt());
}

bool ColourCombo::isBright() const
{
    return currentData(BrightRole).toBool();
}

void ColourCombo::addItem(const QString & text, Spectrum::DisplayDevice::Colour colour)
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

void ColourCombo::setColour(DisplayDevice::Colour colour, bool bright)
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

#include <iostream>

int ColourCombo::findItem(Spectrum::DisplayDevice::Colour colour, bool bright)
{
    std::cout << "Looking for colour " << static_cast<std::uint8_t>(colour) << (bright ? " (bright)" : "") << "\n";

    for (int idx = 0; idx < count(); ++idx) {
        std::cout << "Colour at index " << idx << " id " << itemData(idx, ColourRole).toUInt() << (itemData(idx, BrightRole).toBool() ? " (bright)" : "") << "\n";

        if (static_cast<DisplayDevice::Colour>(itemData(idx, ColourRole).toUInt()) == colour
        && itemData(idx, BrightRole).toBool() == bright) {
            return idx;
        }
    }

    return -1;
}
