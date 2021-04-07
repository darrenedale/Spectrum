//
// Created by darren on 06/04/2021.
//

#include "spectrum16kmemory.h"

using namespace Spectrum;

// NOTE we must report the size as 64kb as the Z80 will address it (and the ROM memory test routing will write there),
// but we only provide a 32kb buffer since we have ensured writes above 32kb are no-ops by overriding the apporopriate
// methods.
Spectrum16kMemory::Spectrum16kMemory()
: BaseSpectrum::MemoryType(0x10000, new Byte[0x8000])
{}

Spectrum16kMemory::~Spectrum16kMemory() = default;

void Spectrum16kMemory::clear()
{
    std::memset(mapAddress(0), 0, 0x8000);
}
