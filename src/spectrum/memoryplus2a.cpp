//
// Created by darren on 06/04/2021.
//

#include <iostream>
#include "memoryplus2a.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;

namespace
{
    // four special configurations of memory paging (the banks that are paged into the address space for each config)
    constexpr const std::uint8_t SpecialMemoryConfigurations[4][4] = {
        {0, 1, 2, 3,},      // config 0
        {4, 5, 6, 7,},      // config 1
        {4, 5, 6, 3,},      // config 2
        {4, 7, 6, 3,},      // config 3
    };
}

MemoryPlus2a::MemoryPlus2a()
: PagingMemory(),
  m_pagingMode(PagingMode::Normal),
  m_specialPagingConfiguration(SpecialPagingConfiguration::Config1)
{}

MemoryPlus2a::~MemoryPlus2a() = default;

MemoryPlus2a::Byte * MemoryPlus2a::mapAddress(Address address) const
{
    if (PagingMode::Special == pagingMode()) {
        auto configIdx = static_cast<int>(specialPagingConfiguration());
        const Byte * base;

        if (address < 0x4000) {
            base = m_ramPages[SpecialMemoryConfigurations[configIdx][0]].data();
        } else if (address < 0x8000) {
            base = m_ramPages[SpecialMemoryConfigurations[configIdx][1]].data();
            address -= 0x4000;
        } else if (address < 0xc000) {
            base = m_ramPages[SpecialMemoryConfigurations[configIdx][2]].data();
            address -= 0x8000;
        } else {
            base = m_ramPages[SpecialMemoryConfigurations[configIdx][3]].data();
            address -= 0xc000;
        }

        return const_cast<Byte *>(base + address);
    }

    return PagingMemory::mapAddress(address);
}

std::unique_ptr<Memory<MemoryPlus2a::Byte>> MemoryPlus2a::clone() const
{
    auto ret = std::make_unique<MemoryPlus2a>();

    // NOTE we'd get better performance out of std::memcpy() because the array copy constructor iterates over the array
    // and invokes the copy constructor for each element. since memory cloning is never (and should never) be used in
    // performance-sensitive code paths we don't need to squeeze out the performance, so we stick with idiomatic code
    ret->m_ramPages = m_ramPages;
    ret->m_roms = m_roms;
    ret->m_pagingMode = m_pagingMode;
    ret->m_romNumber = m_romNumber;
    ret->m_pagedRam = m_pagedRam;
    ret->m_specialPagingConfiguration = m_specialPagingConfiguration;
    return ret;
}
