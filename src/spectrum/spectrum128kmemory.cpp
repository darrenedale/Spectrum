//
// Created by darren on 06/04/2021.
//

#include <iostream>
#include <fstream>
#include "spectrum128kmemory.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;

namespace
{
    constexpr const int RomSize = 0x4000;
    constexpr const int BankSize = 0x4000;

    // the address space mapping
    constexpr const Spectrum128KMemory::Address RomBase = 0x0000;
    constexpr const Spectrum128KMemory::Address RomTop = RomBase + RomSize - 1;
    constexpr const Spectrum128KMemory::Address Bank5Base = 0x4000;
    constexpr const Spectrum128KMemory::Address Bank5Top = Bank5Base + BankSize - 1;
    constexpr const Spectrum128KMemory::Address Bank2Base = 0x8000;
    constexpr const Spectrum128KMemory::Address Bank2Top = Bank2Base + BankSize - 1;
    constexpr const Spectrum128KMemory::Address PagedRamBase = 0xc000;
    constexpr const Spectrum128KMemory::Address PagedRamTop = PagedRamBase + BankSize - 1;
}

Spectrum128KMemory::Spectrum128KMemory()
: BaseSpectrum::MemoryType(0x10000, nullptr),
  m_romNumber(RomNumber::Rom0),
  m_pagedBank(BankNumber::Bank0),
  m_roms(),
  m_ramBanks()
{}

Spectrum128KMemory::~Spectrum128KMemory() = default;

Spectrum128KMemory::Byte * Spectrum128KMemory::mapAddress(Memory::Address address) const
{
    if (address <= RomTop) {
        return const_cast<Byte *>(currentRomPointer() + address);
    }

    if (address >= PagedRamBase) {
        return const_cast<Byte *>(currentPagedBankPointer() + address - PagedRamBase);
    }

    if (address >= Bank2Base) {
        return const_cast<Byte *>(m_ramBanks[static_cast<int>(BankNumber::Bank2)] + address - Bank2Base);
    }

    if (address >= Bank5Base) {
        return const_cast<Byte *>(m_ramBanks[static_cast<int>(BankNumber::Bank5)] + address - Bank5Base);
    }

    // unreachable code
    assert(false);
}

bool Spectrum128KMemory::loadRom(const std::string & fileName, RomNumber romNumber)
{
    std::ifstream in(fileName);

    if (!in) {
        std::cerr << "failed to open 128k ROM image file \"" << fileName << "\"\n";
        return false;
    }

    in.read(reinterpret_cast<std::ifstream::char_type *>(m_roms[static_cast<int>(romNumber)]), RomSize);

    if (in.fail() && !in.eof()) {
        std::cerr << "failed to read 128k ROM image file \"" << fileName << "\"\n";
        return false;
    }

    return true;
}

void Spectrum128KMemory::clear()
{
    for (auto & rom : m_roms) {
        std::memset(rom, 0, RomSize);
    }

    for (auto & bank : m_ramBanks) {
        std::memset(bank, 0, BankSize);
    }
}

void Spectrum128KMemory::readFromBank(BankNumber bank, Byte * buffer, UnsignedWord size, UnsignedWord offset)
{
    assert(offset + size < BankSize);
    std::memcpy(buffer, m_ramBanks[static_cast<int>(bank)] + offset, size);
}

void Spectrum128KMemory::writeToBank(BankNumber bank, Byte * data, UnsignedWord size, UnsignedWord offset)
{
    assert(offset + size < BankSize);
    std::memcpy(m_ramBanks[static_cast<int>(bank)] + offset, data, size);
}
