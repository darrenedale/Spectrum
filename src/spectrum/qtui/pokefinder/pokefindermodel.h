//
// Created by darren on 07/10/2026.
//

#ifndef SPECTRUM_QTUI_POKEFINDERMODEL_H
#define SPECTRUM_QTUI_POKEFINDERMODEL_H

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include "../../pokefinder.h"


namespace Spectrum::QtUi::PokeFinder
{
    class PokeFinderModel
    : public QStandardItemModel
    {
        Q_OBJECT

        public:
            static constexpr int AddressRole = Qt::UserRole + 1;
            static constexpr int ValueRole = Qt::UserRole + 2;
            static constexpr int IsMatchRole = Qt::UserRole + 3;

            void setBeforeAddresses(Spectrum::PokeFinder::Word, const Spectrum::PokeFinder::Addresses &) noexcept;
            void setAfterAddresses(Spectrum::PokeFinder::Word, const Spectrum::PokeFinder::Addresses &) noexcept;
    };
} // QtUi::Spectrum

#endif // SPECTRUM_QTUI_POKEFINDERMODEL_H
