//
// Created by darren on 07/10/2026.
//

#include "pokefindermodel.h"

using namespace Spectrum::QtUi::PokeFinder;

void PokeFinderModel::setBeforeAddresses(const Spectrum::PokeFinder::Word value, const Spectrum::PokeFinder::Addresses& addresses) noexcept
{
    removeRows(0, rowCount());

    for (int row = 0; const auto & address : addresses) {
        setItem(row, 0, new QStandardItem(QString::fromStdString(std::format("{:#06x}", address))));
        setItem(row, 1, new QStandardItem(QString::number(value)));
        setItem(row, 2, new QStandardItem(""));
        setData(index(row, 2), QVariant(static_cast<qulonglong>(address)), AddressRole);
        setData(index(row, 2), QVariant(false), IsMatchRole);
        ++row;
    }
}

void PokeFinderModel::setAfterAddresses(const Spectrum::PokeFinder::Word value, const Spectrum::PokeFinder::Addresses& addresses) noexcept
{
    const auto valueDisplay = QString::number(value);
    const auto noMatchDisplay = tr("No match");

    // grab an item if possible so that we only have to set the fonts once
    const auto * item = this->item(0, 2);

    if (!item) {
        return;
    }

    auto matchItemFont = item->font();
    auto unMatchItemFont = matchItemFont;
    unMatchItemFont.setItalic(false);
    unMatchItemFont.setItalic(true);

    for (int row = 0; row < rowCount(); ++row) {
        auto * item = this->item(row, 2);

        if (std::ranges::contains(addresses, item->data(AddressRole).toUInt())) {
            item->setFont(matchItemFont);
            item->setText(valueDisplay);
            setData(index(row, 2), QVariant(true), IsMatchRole);
            setData(index(row, 2), QVariant(value), ValueRole);
        } else {
            item->setFont(unMatchItemFont);
            item->setText(noMatchDisplay);
            setData(index(row, 2), QVariant(false), IsMatchRole);
        }
    }
}
