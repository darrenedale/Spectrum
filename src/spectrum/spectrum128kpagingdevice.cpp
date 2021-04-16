//
// Created by darren on 06/04/2021.
//

#include "spectrum128k.h"
#include "spectrum128kpagingdevice.h"
#include "spectrum128kmemory.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;

namespace
{
    // mask to apply to the written byte to get the bank to page in
    constexpr const UnsignedByte RamBankMask = 0x07;

    // the bit and mask to apply to the written byte to determine whether the main or 48k ROM should be paged in
    constexpr const UnsignedByte RomNumberBit = 4;
    constexpr const UnsignedByte RomNumberMask = 1 << RomNumberBit;

    // the bit and mask to apply to the written byte to determine whether the shadow screen buffer should be used
    constexpr const UnsignedByte ScreenBufferBit = 3;
    constexpr const UnsignedByte ScreenBufferMask = 1 << ScreenBufferBit;

    // bit and mask to apply to the written byte to determine whether to disable paging
    constexpr const UnsignedByte DisablePagingBit = 5;
    constexpr const UnsignedByte DisablePagingMask = 1 << DisablePagingBit;
}

Spectrum128KPagingDevice::~Spectrum128KPagingDevice() = default;

void Spectrum128KPagingDevice::writeByte(UnsignedWord port, UnsignedByte value)
{
    if (!pagingEnabled()) {
        return;
    }

    auto * memory = dynamic_cast<Spectrum128kMemory *>(spectrum().memory());
    assert(memory);

    // ram bank to page is in bits 0-2
    auto ramBank = static_cast<Spectrum128kMemory::BankNumber>(value & RamBankMask);
    memory->pageBank(ramBank);

    // rom number is in bit 4
    auto rom = static_cast<Spectrum128kMemory::RomNumber>((value & RomNumberMask) >> RomNumberBit);
    memory->pageRom(rom);

    auto screenBuffer = (value & ScreenBufferMask) ? Spectrum128k::ScreenBuffer::Shadow : Spectrum128k::ScreenBuffer::Normal;
    spectrum().setScreenBuffer(screenBuffer);

    if (value & DisablePagingMask) {
        setPagingEnabled(false);
    }
}
