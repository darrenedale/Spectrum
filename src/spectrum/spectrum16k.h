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

        /**
         * It's safe to use the returned pointer as a pointer to a contiguous block of 16kb of Spectrum RAM because
         * this page is never paged out from the address 0x4000.
         */
        [[nodiscard]] ::Z80::UnsignedByte * displayMemory() const override
        {
            return memory()->pointerTo(DisplayMemoryOffset);
        }

        [[nodiscard]] int displayMemorySize() const override
        {
            return DisplayMemorySize;
        }
    };
}

#endif //SPECTRUM_SPECTRUM16K_H
