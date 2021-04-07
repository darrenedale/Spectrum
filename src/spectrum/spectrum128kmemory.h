//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_SPECTRUM128KMEMORY_H
#define SPECTRUM_SPECTRUM128KMEMORY_H

#include "basespectrum.h"
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
    class Spectrum128KMemory
    : public BaseSpectrum::MemoryType
    {
    public:
        /**
         * The size of a RAM bank and a ROM.
         */
        static constexpr const int BankSize = 0x4000;

        /**
         * Enumeration of the available ROMs.
         */
        enum class RomNumber : std::uint8_t
        {
            Rom0 = 0,
            Rom1,
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

        Spectrum128KMemory();
        Spectrum128KMemory(const Spectrum128KMemory &) = delete;
        Spectrum128KMemory(Spectrum128KMemory &&) = delete;
        void operator=(const Spectrum128KMemory &) = delete;
        void operator=(Spectrum128KMemory &&) = delete;
        ~Spectrum128KMemory() override;

        /**
         * Clear the memory.
         *
         * All banks and all ROMs are cleared.
         */
        void clear() override;

        /**
         * Determine the currently paged-in ROM.
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
         * @return
         */
        [[nodiscard]] inline BankNumber currentPagedBank() const
        {
            return m_pagedBank;
        }

        /**
         * Page in the specified ROM.
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
         * @param bank
         */
        inline void pageBank(BankNumber bank)
        {
            m_pagedBank = bank;
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
            return m_ramBanks[static_cast<int>(bank)];
        }

        /**
         * Fetch a pointer to the fist byte in the specified bank of RAM.
         *
         * @return
         */
        Byte * bankPointer(BankNumber bank)
        {
            return m_ramBanks[static_cast<int>(bank)];
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
            return m_roms[static_cast<int>(rom)];
        }

        /**
         * Fetch a pointer to the fist byte in the specified ROM.
         *
         * @return
         */
        Byte * romPointer(RomNumber rom)
        {
            return m_roms[static_cast<int>(rom)];
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
        using BankStorage = Byte[BankSize];

        /**
         * The currently paged-in ROM.
         */
        RomNumber m_romNumber;

        /**
         * Storage for the ROMs.
         */
        BankStorage m_roms[2];

        /**
         * The currently paged-in RAM bank.
         */
        BankNumber m_pagedBank;

        /**
         * Storage for the RAM banks.
         */
        BankStorage m_ramBanks[8];
    };
}

#endif //SPECTRUM_SPECTRUM128KMEMORY_H
