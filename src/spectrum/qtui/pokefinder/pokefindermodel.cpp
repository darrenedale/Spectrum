//
// Created by darren on 07/10/2026.
//

#include "pokefindermodel.h"

#include "../../../util/debug.h"

using namespace Spectrum::QtUi::PokeFinder;

void PokeFinderModel::setBeforeAddresses(const Spectrum::PokeFinder::Word value, const Spectrum::PokeFinder::Addresses& addresses) noexcept
{
    removeRows(0, rowCount());
#if !defined(NDEBUG)
    const auto start = std::chrono::steady_clock::now();
#endif

    for (const auto & address : addresses) {
        auto * column3Item = new QStandardItem("");
        column3Item->setData(QVariant(static_cast<qulonglong>(address)), AddressRole);
        column3Item->setData(QVariant(false), IsMatchRole);

        appendRow({
            new QStandardItem(QString::fromStdString(std::format("{:#06x}", address))),
            new QStandardItem(QString::number(value)),
            column3Item,
        });
    }

#if !defined(NDEBUG)
    Util::debugln(
        "Added {} items for before matches to {} rows in {} ms",
        3 * addresses.size(),
        addresses.size(),
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
    );
#endif
}

void PokeFinderModel::setAfterAddresses(const Spectrum::PokeFinder::Word value, const Spectrum::PokeFinder::Addresses& addresses) noexcept
{
    const auto valueDisplay = QString::number(value);
    const auto noMatchDisplay = tr("No match");

#if !defined(NDEBUG)
    const auto start = std::chrono::steady_clock::now();
#endif

    for (int row = 0; row < rowCount(); ++row) {
        auto * item = this->item(row, 2);

        if (std::ranges::contains(addresses, item->data(AddressRole).toUInt())) {
            item->setText(valueDisplay);
            setData(index(row, 2), QVariant(true), IsMatchRole);
            setData(index(row, 2), QVariant(value), ValueRole);
        } else {
            item->setText(noMatchDisplay);
            setData(index(row, 2), QVariant(false), IsMatchRole);
        }
    }

#if !defined(NDEBUG)
    Util::debugln(
        "Flagged {} of {} addresses as matches in {} ms",
        addresses.size(),
        rowCount(),
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
    );
#endif
}
