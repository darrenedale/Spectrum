//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_SPECTRUMPLUS2AKMEMORY_H
#define SPECTRUM_SPECTRUMPLUS2AKMEMORY_H

#include "types.h"
#include "pagingmemory.h"
#include "../z80/types.h"

namespace Spectrum
{
    /**
     * Models the memory of a Spectrum+2a/+3.
     *
     * The memory consists of the standard Z80 16-bit address space and the standard Spectrum 16K of ROM mapped to the first 16K of that space, followed by 48K
     * of addressable RAM. The ROM space can be occupied by one of four ROMs, which can be paged in and out using standard Z80 OUT instructions (see
     * SpectrumPlus2aPagingDevice). The RAM space is made up of a combination of three out of eight available banks of 16K each. The first 32K of RAM (0x4000 to
     * 0xcfff) if static, made up of banks 5 and 2 (in that order). The final 16K of RAM can use any of the 8 RAM banks, including banks 2 and 5 (in which case
     * the bank will appear both in the static section and the paged section).
     *
     * In addition, this memory model implements a special paging mode, in which case one of four fixed configurations of the memory pages is mapped into the
     * four 16kb chunks of the 64kb address space. Again, this is controlled by standard Z80 OUT instructions.
     */
    class SpectrumPlus2aMemory
    : public PagingMemory<4, 8>
    {
    public:
        /**
         * Initialise a new instance of the Spectrum +2a memory model.
         */
        SpectrumPlus2aMemory();

        /**
         * Copy construction is disabled.
         */
        SpectrumPlus2aMemory(const SpectrumPlus2aMemory &) = delete;

        /**
         * Move construction is disabled.
         */
        SpectrumPlus2aMemory(SpectrumPlus2aMemory &&) = delete;

        /**
         * Copy assignment is disabled.
         */
        void operator=(const SpectrumPlus2aMemory &) = delete;

        /**
         * Move assignment is disabled.
         */
        void operator=(SpectrumPlus2aMemory &&) = delete;

        /**
         * Destructor.
         */
        ~SpectrumPlus2aMemory() override;

        /**
         * Create a clone of the memory.
         *
         * This is guaranteed not to fail unless the required 160kb or so of memory can't be allocated on the host machine.
         *
         * @return An identical copy of this memory object.
         */
        [[nodiscard]] std::unique_ptr<Memory> clone() const override;

        /**
         * Fetch the current paging mode.
         *
         * @return The current paging mode, Normal or Special.
         */
        [[nodiscard]] inline PagingMode pagingMode() const
        {
            return m_pagingMode;
        }

        /**
         * Set the current paging mode.
         *
         * @param mode The paging mode to set.
         */
        inline void setPagingMode(PagingMode mode)
        {
            m_pagingMode = mode;
        }

        /**
         * Determine the currently special paging configuration.
         *
         * This is only relevant if the paging mode is Special, but it will always return one of the special paging configurations.
         *
         * @return The special paging configuration in use (if the memory is in special paging mode).
         */
        [[nodiscard]] inline SpecialPagingConfiguration specialPagingConfiguration() const
        {
            return m_specialPagingConfiguration;
        }

        /**
         * Set the special paging configuration.
         *
         * The configuration set will be used if the memory is in, or is later put into, special paging mode.
         *
         * @param config The configuration to set.
         */
        inline void setSpecialPagingConfiguration(SpecialPagingConfiguration config)
        {
            m_specialPagingConfiguration = config;
        }

    protected:
        /**
         * Map an emulated memory address to a physical address inside one of the ROMs/RAM banks.
         *
         * Reimplemented to support special paging mode.
         *
         * @param address The address to map.
         *
         * @return
         */
        [[nodiscard]] Byte * mapAddress(Address address) const override;

    private:
        /**
         * The current paging mode.
         */
        PagingMode m_pagingMode;

        /**
         * Which configuration is in use for Special paging mode.
         */
        SpecialPagingConfiguration m_specialPagingConfiguration;
    };
}

#endif //SPECTRUM_SPECTRUMPLUS2AKMEMORY_H
