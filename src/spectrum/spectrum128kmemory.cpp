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

using MemoryType = BaseSpectrum::MemoryType;

Spectrum128KMemory::Spectrum128KMemory()
: MemoryType(0x10000),
  m_romNumber(RomNumber::Rom0),
  m_pagedBank(BankNumber::Bank0),
  m_roms(),
  m_ramBanks()
{}

Spectrum128KMemory::~Spectrum128KMemory() = default;

Spectrum128KMemory::Byte * Spectrum128KMemory::mapAddress(MemoryType::Address address) const
{
    if (address <= RomTop) {
        return const_cast<Byte *>(currentRomPointer() + address);
    }

    if (address >= PagedRamBase) {
        return const_cast<Byte *>(currentPagedBankPointer() + address - PagedRamBase);
    }

    if (address >= Bank2Base) {
        return const_cast<Byte *>(m_ramBanks[static_cast<int>(BankNumber::Bank2)].data() + address - Bank2Base);
    }

    if (address >= Bank5Base) {
        return const_cast<Byte *>(m_ramBanks[static_cast<int>(BankNumber::Bank5)].data() + address - Bank5Base);
    }

    // unreachable code
    assert(false);
    return nullptr;
}

bool Spectrum128KMemory::loadRom(const std::string & fileName, RomNumber romNumber)
{
    std::ifstream in(fileName, std::ios::binary | std::ios::in);

    if (!in) {
        std::cerr << "failed to open ROM image file \"" << fileName << "\"\n";
        return false;
    }

    in.read(reinterpret_cast<std::ifstream::char_type *>(m_roms[static_cast<int>(romNumber)].data()), RomSize);

    if (in.fail() && !in.eof()) {
        std::cerr << "failed to read ROM image file \"" << fileName << "\"\n";
        return false;
    }

    return true;
}

void Spectrum128KMemory::clear()
{
    for (auto & rom : m_roms) {
        rom.fill(0);
    }

    for (auto & bank : m_ramBanks) {
        bank.fill(0);
    }
}

std::unique_ptr<Memory<Spectrum128KMemory::Byte>> Spectrum128KMemory::clone() const
{
    auto ret = std::make_unique<Spectrum128KMemory>();

    // NOTE we'd get better performance out of std::memcpy() because the array copy constructor iterates over the array
    // and invokes the copy constructor for each element. since memory cloning is never (and should never) be used in
    // performance-sensitive code paths we don't need to squeeze out the performance, so we stick with idiomatic code
    ret->m_ramBanks = m_ramBanks;
    ret->m_roms = m_roms;
    ret->m_romNumber = m_romNumber;
    ret->m_pagedBank = m_pagedBank;
    return ret;
}

void Spectrum128KMemory::readFromBank(BankNumber bank, Byte * buffer, UnsignedWord size, UnsignedWord offset)
{
    assert(offset + size <= BankSize);
    std::memcpy(buffer, m_ramBanks[static_cast<int>(bank)].data() + offset, size);
}

void Spectrum128KMemory::writeToBank(BankNumber bank, Byte * data, UnsignedWord size, UnsignedWord offset)
{
    assert(offset + size <= BankSize);
    std::memcpy(m_ramBanks[static_cast<int>(bank)].data() + offset, data, size);
}
