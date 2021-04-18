//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_MEMORY128K_H
#define SPECTRUM_MEMORY128K_H

#include "pagingmemory.h"
#include "../z80/types.h"

namespace Spectrum
{
    /**
     * Models the memory of a 128k spectrum.
     *
     * The memory consists of the standard Z80 16-bit address space and the standard Spectrum 16K of ROM mapped to the
     * first 16K of that space, followed by 48K of addressable RAM. The ROM space can be occupied by one of two ROMs,
     * which can be paged in and out using standard Z80 OUT instructions (see Spectrum128kPagingDevice). The RAM space
     * is made up of a combination of three out of eight available banks of 16K each. The first 32K of RAM (0x4000 to
     * 0xcfff) if static, made up of banks 5 and 2 (in that order). The final 16K of RAM can use any of the 8 RAM banks,
     * including banks 2 and 5 (in which case the bank will appear both in the static section and the paged section).
     */
    class Memory128k
    : public PagingMemory<2, 8>
    {
    public:
        /**
         * Initialise a new instance of the Spectrum 128K memory model.
         */
        Memory128k();

        /**
         * Copy construction is disabled.
         */
        Memory128k(const Memory128k &) = delete;

        /**
         * Move construction is disabled.
         */
        Memory128k(Memory128k &&) = delete;

        /**
         * Copy assignment is disabled.
         */
        void operator=(const Memory128k &) = delete;

        /**
         * Move assignment is disabled.
         */
        void operator=(Memory128k &&) = delete;

        /**
         * Destructor.
         */
        ~Memory128k() override;

        /**
         * Create a clone of the memory.
         *
         * This is guaranteed not to fail unless the required 160kb or so of memory can't be allocated on the host machine.
         *
         * @return An identical copy of this memory object.
         */
        [[nodiscard]] std::unique_ptr<Memory> clone() const override;
    };
}

#endif //SPECTRUM_MEMORY128K_H
