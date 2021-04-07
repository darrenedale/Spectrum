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
     * Representation of the memory for a Spectrum 16K.
     *
     * The memory is a single, linear block of 32kb. The first 16kb is ROM (The Spectrum 16K uses the same ROM as the
     * Spectrum 48K), the remaining 16Kb is RAM. The remaining 32Kb of the Z80 address space remains addressable, and is
     * referenced in the Spectrum ROM (notably when it scans the address space to work out how much actual memory is
     * available). Therefore reads and writes to the top 32kb must be accepted but do nothing. For this reason, size()
     * reports 64kb, but only 32kb of storage is actually allocated.
     *
     * Don't use pointer arithmetic or operator[] to read or write above 32kb, you'll get a segfault (if you're lucky).
     */
    class Spectrum16kMemory
    : public BaseSpectrum::MemoryType
    {
    public:
        /**
         * Initialise a new memory object for a Spectrum 16K.
         */
        Spectrum16kMemory();

        // Spectrum16kMemory objects are not copy or move constructable or assignable
        Spectrum16kMemory(const Spectrum16kMemory &) = delete;
        Spectrum16kMemory(Spectrum16kMemory &&) = delete;
        void operator=(const Spectrum16kMemory &) = delete;
        void operator=(Spectrum16kMemory &&) = delete;

        ~Spectrum16kMemory() override;

        /**
         * Clear the memory.
         */
        void clear() override;

        /**
         * Read a single byte from a given address.
         *
         * If the address is above 0x7fff, the read will always provide 0xff.
         *
         * @param address
         * @return
         */
        [[nodiscard]] Byte readByte(Address address) const override
        {
            if (address & 0x8000) {
                return 0xff;
            }

            return *mapAddress(address);
        }

        /**
         * Write a single byte to a given address.
         *
         * If the address is above 0x7fff, the write is a no-op.
         *
         * @param address
         * @return
         */
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
