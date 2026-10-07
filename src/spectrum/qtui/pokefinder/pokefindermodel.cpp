//
// Created by darren on 07/10/2026.
//

#include "pokefindermodel.h"

using namespace Spectrum::QtUi::PokeFinder;

namespace
{
    constexpr int AddressRole = Qt::UserRole + 1;
}

PokeFinderModel::Items PokeFinderModel::findAddressItems(const Spectrum::PokeFinder::Address address) const noexcept
{
    Items items(rowCount());

    for (int row = 0; row < rowCount(); ++row) {
        auto * item = this->item(row, 2);

        if (item->data(AddressRole).toUInt() == address) {
            items.push_back(item);
        }
    }

    return items;
}

void PokeFinderModel::setBeforeAddresses(const Spectrum::PokeFinder::Word lives, const Spectrum::PokeFinder::Addresses& addresses) noexcept
{
    removeRows(0, rowCount());

    for (int row = 0; const auto & address : addresses) {
        setItem(row, 0, new QStandardItem(QString::fromStdString(std::format("{:#06x}", address))));
        setItem(row, 1, new QStandardItem(QString::number(lives)));
        setItem(row, 2, new QStandardItem(""));
        setData(index(row, 2), QVariant(static_cast<qulonglong>(address)), AddressRole);
        ++row;
    }
}

void PokeFinderModel::setAfterAddresses(const Spectrum::PokeFinder::Word lives, const Spectrum::PokeFinder::Addresses& addresses) noexcept
{
    const auto livesDisplay = QString::number(lives);
    const auto noMatchDisplay = tr("No match");

    for (int row = 0; row < rowCount(); ++row) {
        auto * item = this->item(row, 2);

        if (std::ranges::contains(addresses, item->data(AddressRole).toUInt())) {
            auto font = item->font();
            font.setItalic(false);
            item->setFont(font);
            item->setText(livesDisplay);
        } else {
            auto font = item->font();
            font.setItalic(true);
            item->setFont(font);
            item->setText(noMatchDisplay);
        }
    }
}
