//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_SPECTRUMPLUS2AKMEMORY_H
#define SPECTRUM_SPECTRUMPLUS2AKMEMORY_H

#include "basespectrum.h"
#include "../z80/types.h"

namespace Spectrum
{
    /**
     * Models the memory of a Spectrum+2a/+3.
     *
     * The memory consists of the standard Z80 16-bit address space and the standard Spectrum 16K of ROM mapped to the
     * first 16K of that space, followed by 48K of addressable RAM. The ROM space can be occupied by one of four ROMs,
     * which can be paged in and out using standard Z80 OUT instructions (see SpectrumPlus2aPagingDevice). The RAM space
     * is made up of a combination of three out of eight available banks of 16K each. The first 32K of RAM (0x4000 to
     * 0xcfff) if static, made up of banks 5 and 2 (in that order). The final 16K of RAM can use any of the 8 RAM banks,
     * including banks 2 and 5 (in which case the bank will appear both in the static section and the paged section).
     */
    class SpectrumPlus2aMemory
    : public Memory<BaseSpectrum::ByteType>
    {
    public:
        /**
         * The size of a RAM bank and a ROM.
         */
        static constexpr const int BankSize = 0x4000;

        enum class PagingMode : std::uint8_t
        {
            Normal,
            Special,
        };

        /**
         * Enumeration of the available ROMs.
         */
        enum class RomNumber : std::uint8_t
        {
            Rom0 = 0,
            Rom1,
            Rom2,
            Rom3,
        };

        /**
         * Enumeration of the available RAM banks.
         */
        enum class BankNumber : std::uint8_t
        {
            Bank0 = 0,
            Bank1,
            Bank2,
            Bank3,
            Bank4,
            Bank5,
            Bank6,
            Bank7,
        };

        enum class SpecialPagingConfiguration : std::uint8_t
        {
            Config1,
            Config2,
            Config3,
            Config4,
        };

        SpectrumPlus2aMemory();
        SpectrumPlus2aMemory(const SpectrumPlus2aMemory &) = delete;
        SpectrumPlus2aMemory(SpectrumPlus2aMemory &&) = delete;
        void operator=(const SpectrumPlus2aMemory &) = delete;
        void operator=(SpectrumPlus2aMemory &&) = delete;
        ~SpectrumPlus2aMemory() override;

        /**
         * Clear the memory.
         *
         * All banks and all ROMs are cleared.
         */
        void clear() override;

        [[nodiscard]] std::unique_ptr<Memory> clone() const override;

        /**
         * Fetch the current paging mode.
         *
         * @return
         */
        [[nodiscard]] inline PagingMode pagingMode() const
        {
            return m_pagingMode;
        }

        /**
         * Set the current paging mode.
         *
         * @param mode
         */
        inline void setPagingMode(PagingMode mode)
        {
            m_pagingMode = mode;
        }

        /**
         * Determine the currently paged-in ROM.
         *
         * This is only relevant if the paging mode is Normal. In Special paging mode, the ROM is not used.
         *
         * @return
         */
        [[nodiscard]] inline RomNumber currentRom() const
        {
            return m_romNumber;
        }

        /**
         * Determine the currently paged-in RAM bank.
         *
         * This is only relevant if the paging mode is Normal. In Special paging mode, the paged-in banks are determined
         * by the special paging mode configuration.
         *
         * @return
         */
        [[nodiscard]] inline BankNumber currentPagedBank() const
        {
            return m_pagedBank;
        }

        /**
         * Determine the currently special paging configuration.
         *
         * This is only relevant if the paging mode is Special..
         *
         * @return
         */
        [[nodiscard]] inline SpecialPagingConfiguration specialPagingConfiguration() const
        {
            return m_specialPagingConfiguration;
        }

        /**
         * Page in the specified ROM.
         *
         * The specified ROM will be paged in when the paging mode is Normal. While the paging mode is Special the
         * configured special paging setup will be in effect.
         *
         * @param rom
         */
        inline void pageRom(RomNumber rom)
        {
            m_romNumber = rom;
        }

        /**
         * Page in the specified RAM bank.
         *
         * The specified bank will be paged in when the paging mode is Normal. While the paging mode is Special the
         * configured special paging setup will be in effect.
         *
         * @param bank
         */
        inline void pageBank(BankNumber bank)
        {
            m_pagedBank = bank;
        }

        /**
         *
         * @param config
         */
        inline void setSpecialPagingConfiguration(SpecialPagingConfiguration config)
        {
            m_specialPagingConfiguration = config;
        }

        /**
         * Load the content of a file into the specified ROM.
         *
         * @param fileName
         * @return
         */
        bool loadRom(const std::string & fileName, RomNumber = RomNumber::Rom0);

        /**
         * Fetch a const pointer to the fist byte in the specified bank of RAM.
         *
         * @return
         */
        [[nodiscard]] const Byte * bankPointer(BankNumber bank) const
        {
            return m_ramBanks[static_cast<int>(bank)].data();
        }

        /**
         * Fetch a pointer to the fist byte in the specified bank of RAM.
         *
         * @return
         */
        Byte * bankPointer(BankNumber bank)
        {
            return m_ramBanks[static_cast<int>(bank)].data();
        }

        /**
         * Fetch a const pointer to the fist byte in the currently paged-in bank of RAM.
         *
         * @return
         */
        [[nodiscard]] inline const Byte * currentPagedBankPointer() const
        {
            return bankPointer(currentPagedBank());
        }

        /**
         * Fetch a pointer to the fist byte in the currently paged-in bank of RAM.
         *
         * @return
         */
        inline Byte * currentPagedBankPointer()
        {
            return bankPointer(currentPagedBank());
        }

        /**
         * Fetch a const pointer to the fist byte in the specified ROM.
         *
         * @return
         */
        [[nodiscard]] const Byte * romPointer(RomNumber rom) const
        {
            return m_roms[static_cast<int>(rom)].data();
        }

        /**
         * Fetch a pointer to the fist byte in the specified ROM.
         *
         * @return
         */
        Byte * romPointer(RomNumber rom)
        {
            return m_roms[static_cast<int>(rom)].data();
        }

        /**
        * Fetch a const pointer to the fist byte in the currently paged-in ROM.
        *
        * @return
        */
        [[nodiscard]] inline const Byte * currentRomPointer() const
        {
            return romPointer(currentRom());
        }

        /**
         * Fetch a pointer to the fist byte in the currently paged-in ROM.
         *
         * @return
         */
        inline Byte * currentRomPointer()
        {
            return romPointer(currentRom());
        }

        /**
         * Write directly into a specified memory bank.
         *
         * Useful when reading snapshots, for example.
         *
         * @param bank The bank to write to
         * @param data The data to write
         * @param size The number of bytes to write. Defaults to a full bank.
         * @param offset The offset into the bank at which to start writing. Defaults to the first byte in the bank.
         */
        void writeToBank(BankNumber bank, Byte * data, ::Z80::UnsignedWord size = BankSize, ::Z80::UnsignedWord offset = 0);

        /**
         * Read directly from a specified memory bank.
         *
         * Useful when writing snapshots, for example.
         *
         * @param bank The bank to read from
         * @param data The buffer into which to read the data. Must be big enough to hold size bytes.
         * @param size The number of bytes to read. Defaults to a full bank.
         * @param offset The offset into the bank at which to start reading. Defaults to the first byte in the bank.
         */
        void readFromBank(BankNumber, Byte *, ::Z80::UnsignedWord size = BankSize, ::Z80::UnsignedWord offset = 0);

    protected:
        /**
         * Map an emulated memory address to a physical address inside one of the ROMs/RAM banks.
         *
         * @param address
         * @return
         */
        [[nodiscard]] Byte * mapAddress(Address address) const override;

    private:
        using BankStorage = std::array<Byte, BankSize>;

        /**
         * The current paging mode.
         */
        PagingMode m_pagingMode;

        /**
         * The currently paged-in ROM.
         */
        RomNumber m_romNumber;

        /**
         * Storage for the ROMs.
         */
        std::array<BankStorage, 4> m_roms;

        /**
         * The currently paged-in RAM bank.
         */
        BankNumber m_pagedBank;

        /**
         * Storage for the RAM banks.
         */
        std::array<BankStorage, 8> m_ramBanks;

        /**
         * Which configuration is in use for Special pagign mode.
         */
        SpecialPagingConfiguration m_specialPagingConfiguration;
    };
}

#endif //SPECTRUM_SPECTRUMPLUS2AKMEMORY_H
