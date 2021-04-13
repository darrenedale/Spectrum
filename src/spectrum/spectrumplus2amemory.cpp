//
// Created by darren on 06/04/2021.
//

#include <iostream>
#include <fstream>
#include "spectrumplus2amemory.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;

namespace
{
    constexpr const int RomSize = 0x4000;
    constexpr const int BankSize = 0x4000;

    // the address space mapping
    constexpr const SpectrumPlus2aMemory::Address RomBase = 0x0000;
    constexpr const SpectrumPlus2aMemory::Address RomTop = RomBase + RomSize - 1;
    constexpr const SpectrumPlus2aMemory::Address Bank5Base = 0x4000;
    constexpr const SpectrumPlus2aMemory::Address Bank5Top = Bank5Base + BankSize - 1;
    constexpr const SpectrumPlus2aMemory::Address Bank2Base = 0x8000;
    constexpr const SpectrumPlus2aMemory::Address Bank2Top = Bank2Base + BankSize - 1;
    constexpr const SpectrumPlus2aMemory::Address PagedRamBase = 0xc000;
    constexpr const SpectrumPlus2aMemory::Address PagedRamTop = PagedRamBase + BankSize - 1;

    // four special configurations of memory paging (the banks that are paged into the address space for each config)
    constexpr const std::uint8_t SpecialMemoryConfigurations[4][4] = {
        {0, 1, 2, 3,},      // config 0
        {4, 5, 6, 7,},      // config 1
        {4, 5, 6, 3,},      // config 2
        {4, 7, 6, 3,},      // config 3
    };
}

using MemoryType = BaseSpectrum::MemoryType;

SpectrumPlus2aMemory::SpectrumPlus2aMemory()
: MemoryType(0x10000),
  m_pagingMode(PagingMode::Normal),
  m_romNumber(RomNumber::Rom0),
  m_pagedBank(BankNumber::Bank0),
  m_roms(),
  m_ramBanks(),
  m_specialPagingConfiguration(SpecialPagingConfiguration::Config1)
{}

SpectrumPlus2aMemory::~SpectrumPlus2aMemory() = default;

SpectrumPlus2aMemory::Byte * SpectrumPlus2aMemory::mapAddress(MemoryType::Address address) const
{
    if (PagingMode::Special == pagingMode()) {
        auto idx = static_cast<int>(specialPagingConfiguration());
        const Byte * base;

        if (address < 0x4000) {
            base = m_ramBanks[SpecialMemoryConfigurations[idx][0]].data();
        } else if (address < 0x8000) {
            base = m_ramBanks[SpecialMemoryConfigurations[idx][1]].data();
            address -= 0x4000;
        } else if (address < 0xc000) {
            base = m_ramBanks[SpecialMemoryConfigurations[idx][2]].data();
            address -= 0x8000;
        } else {
            base = m_ramBanks[SpecialMemoryConfigurations[idx][3]].data();
            address -= 0xc000;
        }

        return const_cast<Byte *>(base + address);
    } else {
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
    }

    // unreachable code
    assert(false);
    return nullptr;
}

bool SpectrumPlus2aMemory::loadRom(const std::string & fileName, RomNumber romNumber)
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

void SpectrumPlus2aMemory::clear()
{
    for (auto & rom : m_roms) {
        rom.fill(0);
    }

    for (auto & bank : m_ramBanks) {
        bank.fill(0);
    }
}

std::unique_ptr<Memory<SpectrumPlus2aMemory::Byte>> SpectrumPlus2aMemory::clone() const
{
    auto ret = std::make_unique<SpectrumPlus2aMemory>();

    // NOTE we'd get better performance out of std::memcpy() because the array copy constructor iterates over the array
    // and invokes the copy constructor for each element. since memory cloning is never (and should never) be used in
    // performance-sensitive code paths we don't need to squeeze out the performance, so we stick with idiomatic code
    ret->m_ramBanks = m_ramBanks;
    ret->m_roms = m_roms;
    ret->m_pagingMode = m_pagingMode;
    ret->m_romNumber = m_romNumber;
    ret->m_pagedBank = m_pagedBank;
    ret->m_specialPagingConfiguration = m_specialPagingConfiguration;
    return ret;
}

void SpectrumPlus2aMemory::readFromBank(BankNumber bank, Byte * buffer, UnsignedWord size, UnsignedWord offset)
{
    assert(offset + size <= BankSize);
    std::memcpy(buffer, m_ramBanks[static_cast<int>(bank)].data() + offset, size);
}

void SpectrumPlus2aMemory::writeToBank(BankNumber bank, Byte * data, UnsignedWord size, UnsignedWord offset)
{
    assert(offset + size <= BankSize);
    std::memcpy(m_ramBanks[static_cast<int>(bank)].data() + offset, data, size);
}
