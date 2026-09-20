//
// Created by darren on 09/04/2021.
//

#ifndef SPECTRUM_SPECTRUMPLUS3_H
#define SPECTRUM_SPECTRUMPLUS3_H

#include "spectrumplus2a.h"
#include "memory/memoryplus3.h"

namespace Spectrum
{
    /**
     * Spectrum +3 is effectively a Plus2a with a floppy drive.
     *
     * TODO add the floppy drive device when available.
     */
    class SpectrumPlus3
    : public SpectrumPlus2a
    {
    public:
        using MemoryType = Memory::MemoryPlus3;

        /**
         * Import base class constructors.
         */
        using SpectrumPlus2a::SpectrumPlus2a;

        /**
         * Destructor
         */
        ~SpectrumPlus3() override;

        /**
         * The model type.
         *
         * @return always Model::SpectrumPlus3.
         */
        [[nodiscard]]
        constexpr Model model() const noexcept override
        {
            return Model::SpectrumPlus3;
        }
    };
}

#endif //SPECTRUM_SPECTRUMPLUS3_H
