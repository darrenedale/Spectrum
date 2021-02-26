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
    addItem(tr("Black"), SpectrumDisplayDevice::Colour::Black);
    addItem(tr("Blue"), SpectrumDisplayDevice::Colour::Blue);
    addItem(tr("Red"), SpectrumDisplayDevice::Colour::Red);
    addItem(tr("Magenta"), SpectrumDisplayDevice::Colour::Magenta);
    addItem(tr("Green"), SpectrumDisplayDevice::Colour::Green);
    addItem(tr("Cyan"), SpectrumDisplayDevice::Colour::Cyan);
    addItem(tr("Yellow"), SpectrumDisplayDevice::Colour::Yellow);
    addItem(tr("White"), SpectrumDisplayDevice::Colour::White);

    connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this]() {
        Q_EMIT colourSelected(colour(), isBright());
    });
}

ColourCombo::~ColourCombo() = default;

SpectrumDisplayDevice::Colour ColourCombo::colour() const
{
    return static_cast<SpectrumDisplayDevice::Colour>(currentData(ColourRole).toUInt());
}

bool ColourCombo::isBright() const
{
    return currentData(BrightRole).toBool();
}

void ColourCombo::addItem(const QString & text, Spectrum::SpectrumDisplayDevice::Colour colour)
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

void ColourCombo::setColour(SpectrumDisplayDevice::Colour colour, bool bright)
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

int ColourCombo::findItem(Spectrum::SpectrumDisplayDevice::Colour colour, bool bright)
{
    std::cout << "Looking for colour " << static_cast<std::uint8_t>(colour) << (bright ? " (bright)" : "") << "\n";

    for (int idx = 0; idx < count(); ++idx) {
        std::cout << "Colour at index " << idx << " id " << itemData(idx, ColourRole).toUInt() << (itemData(idx, BrightRole).toBool() ? " (bright)" : "") << "\n";

        if (static_cast<SpectrumDisplayDevice::Colour>(itemData(idx, ColourRole).toUInt()) == colour
        && itemData(idx, BrightRole).toBool() == bright) {
            return idx;
        }
    }

    return -1;
}
