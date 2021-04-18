//
// Created by darren on 06/04/2021.
//

#include "spectrum128kmemory.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;

Spectrum128kMemory::Spectrum128kMemory()
: PagingMemory()
{}

Spectrum128kMemory::~Spectrum128kMemory() = default;

std::unique_ptr<Memory<Spectrum128kMemory::Byte>> Spectrum128kMemory::clone() const
{
    auto ret = std::make_unique<Spectrum128kMemory>();

    // NOTE we'd get better performance out of std::memcpy() because the array copy constructor iterates over the array
    // and invokes the copy constructor for each element. since memory cloning is never (and should never) be used in
    // performance-sensitive code paths we don't need to squeeze out the performance, so we stick with idiomatic code
    ret->m_ramPages = m_ramPages;
    ret->m_roms = m_roms;
    ret->m_romNumber = m_romNumber;
    ret->m_pagedRam = m_pagedRam;
    return ret;
}
