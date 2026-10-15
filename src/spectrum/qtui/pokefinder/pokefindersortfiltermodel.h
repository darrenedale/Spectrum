//
// Created by darren on 09/10/2026.
//

#ifndef SPECTRUM_QTUI_POKEFINDERSORTFILTERMODEL_H
#define SPECTRUM_QTUI_POKEFINDERSORTFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace Spectrum::QtUi::PokeFinder
{
    class PokeFinderSortFilterModel
    : public QSortFilterProxyModel
    {
        Q_OBJECT

        public:
            void hideRomAddresses(bool hide) noexcept;

            [[nodiscard]]
            bool romAddressesHidden() const noexcept
            {
                return m_hideRomAddresses;
            }

            void hideNonMatchingAddresses(bool hide) noexcept;

            [[nodiscard]]
            bool nonMatchingAddressesHidden() const noexcept
            {
                return m_hideNonMatchingAddresses;
            }

        protected:
            [[nodiscard]]
            bool filterAcceptsRow(int, const QModelIndex &) const noexcept override;

        private:
            bool m_hideRomAddresses = true;
            bool m_hideNonMatchingAddresses = true;
    };
} // PokeFinder QtUi Spectrum

#endif //SPECTRUM_QTUI_POKEFINDERSORTFILTERMODEL_H
