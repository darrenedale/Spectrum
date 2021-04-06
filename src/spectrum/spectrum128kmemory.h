//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_SPECTRUM128KMEMORY_H
#define SPECTRUM_SPECTRUM128KMEMORY_H

#include "../memory.h"
#include "../z80/types.h"

namespace Spectrum
{
    class Spectrum128KMemory
    : public Memory<Z80::UnsignedByte>
    {
    public:
        enum class RomNumber : std::uint8_t
        {
            Rom0 = 0,
            Rom1,
        };

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

        void clear() override;

        [[nodiscard]] inline RomNumber currentRom() const
        {
            return m_romNumber;
        }

        [[nodiscard]] inline BankNumber currentPagedBank() const
        {
            return m_pagedBank;
        }

        inline void pageBank(BankNumber bank)
        {
            m_pagedBank = bank;
        }

        inline void pageRom(RomNumber rom)
        {
            m_romNumber = rom;
        }

        bool loadRom(const std::string & fileName, RomNumber = RomNumber::Rom0);

        [[nodiscard]] const Byte * bankPointer(BankNumber bank) const
        {
            return m_ramBanks[static_cast<int>(bank)];
        }

        Byte * bankPointer(BankNumber bank)
        {
            return m_ramBanks[static_cast<int>(bank)];
        }

        [[nodiscard]] inline const Byte * currentPagedBankPointer() const
        {
            return bankPointer(currentPagedBank());
        }

        inline Byte * currentPagedBankPointer()
        {
            return bankPointer(currentPagedBank());
        }

        [[nodiscard]] const Byte * romPointer(RomNumber rom) const
        {
            return m_roms[static_cast<int>(rom)];
        }

        Byte * romPointer(RomNumber rom)
        {
            return m_roms[static_cast<int>(rom)];
        }

        [[nodiscard]] inline const Byte * currentRomPointer() const
        {
            return romPointer(currentRom());
        }

        inline Byte * currentRomPointer()
        {
            return romPointer(currentRom());
        }

    protected:
        [[nodiscard]] Byte * mapAddress(Address address) const override;

    private:
        using BankStorage = Byte[0x4000];

        RomNumber m_romNumber;
        BankStorage m_roms[2];

        BankNumber m_pagedBank;
        BankStorage m_ramBanks[8];
    };
}

#endif //SPECTRUM_SPECTRUM128KMEMORY_H
