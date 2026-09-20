//
// Created by darren on 06/04/2021.
//

#include "pagingdevice128k.h"
#include "../memory/memory128k.h"
#include "../spectrum128k.h"

using namespace Spectrum::Devices;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;

namespace
{
    // mask to apply to the written byte to get the bank to page in
    constexpr UnsignedByte RamBankMask = 0x07;

    // the bit and mask to apply to the written byte to determine whether the main or 48k ROM should be paged in
    constexpr UnsignedByte RomNumberBit = 4;
    constexpr UnsignedByte RomNumberMask = 1 << RomNumberBit;

    // the bit and mask to apply to the written byte to determine whether the shadow screen buffer should be used
    constexpr UnsignedByte ScreenBufferBit = 3;
    constexpr UnsignedByte ScreenBufferMask = 1 << ScreenBufferBit;

    // bit and mask to apply to the written byte to determine whether to disable paging
    constexpr UnsignedByte DisablePagingBit = 5;
    constexpr UnsignedByte DisablePagingMask = 1 << DisablePagingBit;
}

PagingDevice128k::~PagingDevice128k() = default;

void PagingDevice128k::writeByte(const UnsignedWord port, const UnsignedByte value)
{
    if (!pagingEnabled()) {
        return;
    }

    auto * memory = dynamic_cast<Memory::Memory128k *>(spectrum().memory());
    assert(memory);

    // ram bank to page is in bits 0-2
    memory->pageRam(value & RamBankMask);

    // rom number is in bit 4
    memory->pageRom((value & RomNumberMask) >> RomNumberBit);

    const auto screenBuffer = (value & ScreenBufferMask) ? Spectrum128k::ScreenBuffer::Shadow : Spectrum128k::ScreenBuffer::Normal;
    spectrum().setScreenBuffer(screenBuffer);

    if (value & DisablePagingMask) {
        setPagingEnabled(false);
    }
}
