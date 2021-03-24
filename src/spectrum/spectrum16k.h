//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_SPECTRUM16K_H
#define SPECTRUM_SPECTRUM16K_H

#include <string>

#include "basespectrum.h"

namespace Spectrum
{
    class Spectrum16k
    : public BaseSpectrum
    {
    public:
        static constexpr const int DisplayMemoryOffset = 0x4000;
        static constexpr const int DisplayMemorySize = 6912;

        Spectrum16k();
        explicit Spectrum16k(const std::string & romFile);
        ~Spectrum16k() override;

        [[nodiscard]] std::uint8_t * displayMemory() const override
        {
            return memory() + DisplayMemoryOffset;
        }

        [[nodiscard]] int displayMemorySize() const override
        {
            return DisplayMemorySize;
        }
    };
}

#endif //SPECTRUM_SPECTRUM16K_H
