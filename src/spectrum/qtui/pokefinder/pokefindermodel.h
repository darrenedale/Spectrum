//
// Created by darren on 07/10/2026.
//

#ifndef SPECTRUM_QTUI_POKEFINDERMODEL_H
#define SPECTRUM_QTUI_POKEFINDERMODEL_H

#include <QStandardItemModel>

#include "../../pokefinder.h"


namespace Spectrum::QtUi::PokeFinder
{
    class PokeFinderModel
    : public QStandardItemModel
    {
        public:
            using Items = std::vector<QStandardItem*>;

            void setBeforeAddresses(Spectrum::PokeFinder::Word, const Spectrum::PokeFinder::Addresses &) noexcept;
            void setAfterAddresses(Spectrum::PokeFinder::Word, const Spectrum::PokeFinder::Addresses &) noexcept;

        private:
            [[nodiscard]]
            Items findAddressItems(Spectrum::PokeFinder::Address address) const noexcept;
    };
} // QtUi::Spectrum

#endif // SPECTRUM_QTUI_POKEFINDERMODEL_H
