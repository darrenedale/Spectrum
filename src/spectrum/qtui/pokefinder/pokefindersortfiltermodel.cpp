//
// Created by darren on 09/10/2026.
//

#include "pokefindersortfiltermodel.h"

#include "../../pokefinder.h"
#include "pokefindermodel.h"

using namespace Spectrum::QtUi::PokeFinder;

using PokeFinderClass = Spectrum::PokeFinder;

void PokeFinderSortFilterModel::hideNonMatchingAddresses(const bool hide) noexcept
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    m_hideNonMatchingAddresses = hide;
    endFilterChange(Direction::Rows);
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_hideNonMatchingAddresses = hide;
    invalidateRowsFilter();
#else
    m_hideNonMatchingAddresses = hide;
    invalidateFilter();
#endif
}

void PokeFinderSortFilterModel::hideRomAddresses(const bool show) noexcept
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    m_hideRomAddresses = show;
    endFilterChange(Direction::Rows);
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_hideNonMatchingAddresses = hide;
    invalidateRowsFilter();
#else
    m_hideRomAddresses = show;
    invalidateFilter();
#endif
}

bool PokeFinderSortFilterModel::filterAcceptsRow(const int sourceRow, const QModelIndex & sourceParent) const noexcept
{
    const auto index = sourceModel()->index(sourceRow, 2, sourceParent);

    return
        // empty means the "after" search hasn't been performed yet
        sourceModel()->data(index, Qt::DisplayRole).value<QString>().isEmpty()
        || (
            (
                !m_hideNonMatchingAddresses
                || sourceModel()->data(index, PokeFinderModel::IsMatchRole).value<bool>()
            ) && (
                !m_hideRomAddresses
                || sourceModel()->data(index, PokeFinderModel::AddressRole).value<PokeFinderClass::Address>() >= 0x4000
            )
        );
}
