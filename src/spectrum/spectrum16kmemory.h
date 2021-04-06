//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_SPECTRUM16KMEMORY_H
#define SPECTRUM_SPECTRUM16KMEMORY_H

#include "basespectrum.h"
#include "../memory.h"

namespace Spectrum
{
    /**
     * NOTE size() will report 64kb out of necessity (Z80 still has a 64k address space), but there is only actually
     * 32kb of storage (16kb ROM, 16KB RAM). Reads and writes above 32kb are ignored. Don't use pointer arithmetic or
     * operator[] to write above 32kb, you'll get a segfault (if you're lucky).
     */
    class Spectrum16KMemory
    : public BaseSpectrum::MemoryType
    {
    public:
        Spectrum16KMemory();
        Spectrum16KMemory(const Spectrum16KMemory &) = delete;
        Spectrum16KMemory(Spectrum16KMemory &&) = delete;
        void operator=(const Spectrum16KMemory &) = delete;
        void operator=(Spectrum16KMemory &&) = delete;
        ~Spectrum16KMemory() override;

        void clear() override;

        [[nodiscard]] Byte readByte(Address address) const override
        {
            if (address & 0x8000) {
                return 0xff;
            }

            return *mapAddress(address);
        }

        void writeByte(Address address, Byte value) override
        {
            if (address & 0x8000) {
                return;
            }

            Memory::writeByte(address, value);
        }
    };
}

#endif //SPECTRUM_SPECTRUM16KMEMORY_H
