//
// Created by darren on 06/04/2021.
//

#include "spectrumplus2a.h"
#include "spectrumplus2apagingdevice.h"
#include "spectrumplus2amemory.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;
using SpecialPagingConfiguration = SpectrumPlus2aMemory::SpecialPagingConfiguration;

namespace
{
    // writes to port 0x7ffd
    // mask to apply to the written byte to get the bank to page in
    constexpr const UnsignedByte RamBankMask = 0x07;

    // the bit and mask to apply to the written byte to determine the low bit of the ROM to page in
    constexpr const UnsignedByte RomNumberLowBit = 4;
    constexpr const UnsignedByte RomNumberLowMask = 1 << RomNumberLowBit;

    // the bit and mask to apply to the written byte to determine whether the shadow screen buffer should be used
    constexpr const UnsignedByte ScreenBufferBit = 3;
    constexpr const UnsignedByte ScreenBufferMask = 1 << ScreenBufferBit;

    // bit and mask to apply to the written byte to determine whether to disable paging
    constexpr const UnsignedByte DisablePagingBit = 5;
    constexpr const UnsignedByte DisablePagingMask = 1 << DisablePagingBit;
    
    // writes to port 0x1ffd
    // the bit and mask to apply to the written byte to switch between normal (0) and special (1) paging mode
    constexpr const UnsignedByte PagingModeBit = 0;
    constexpr const UnsignedByte PagingModeMask = 1 << PagingModeBit;

    // the bit and mask to apply to the written byte to determine the low bit of the ROM to page in
    constexpr const UnsignedByte RomNumberHighBit = 2;
    constexpr const UnsignedByte RomNumberHighMask = 1 << RomNumberHighBit;

    // the bit and mask to apply to the written byte to determine whether the disk motor should be running (1) or idle (0)
    constexpr const UnsignedByte DiskMotorBit = 3;
    constexpr const UnsignedByte DiskMotorMask = 1 << DiskMotorBit;

    // the bit and mask to apply to the written byte to determine the printer port strobe
    constexpr const UnsignedByte PrinterStrobeBit = 4;
    constexpr const UnsignedByte PrinterStrobeMask = 1 << PrinterStrobeBit;
    
    // extract the paging configuration from the byte when bit 0 indicates that special paging mode is in use
    constexpr const UnsignedByte SpecialPagingConfigurationMask = 0b00000110;
    constexpr const UnsignedByte SpecialPagingConfigurationShift = 1;
}

SpectrumPlus2aPagingDevice::~SpectrumPlus2aPagingDevice() = default;

void SpectrumPlus2aPagingDevice::writeByte(UnsignedWord port, UnsignedByte value)
{
    if (!pagingEnabled()) {
        return;
    }
    
    if (0b0111111111111101 == (port | 0b0011111111111101)) {
        writePort7ffd(value);
    } else if (0b0001111111111101 == (port | 0b0000111111111101)) {
        writePort1ffd(value);
    }
}

void SpectrumPlus2aPagingDevice::writePort7ffd(::Z80::UnsignedByte value)
{
    auto * memory = dynamic_cast<SpectrumPlus2aMemory *>(spectrum().memory());
    assert(memory);

    // ram bank to page is in bits 0-2
    auto ramBank = static_cast<SpectrumPlus2aMemory::BankNumber>(value & RamBankMask);
    memory->pageBank(ramBank);

    // rom number low bit is in bit 4; high bit is retained from current ROM
    auto rom = static_cast<SpectrumPlus2aMemory::RomNumber>(
            ((value & RomNumberLowMask) >> RomNumberLowBit)
            | (static_cast<std::uint8_t>(memory->currentRom()) & 0x02)
        );
    memory->pageRom(rom);

    auto screenBuffer = (value & ScreenBufferMask) ? SpectrumPlus2a::ScreenBuffer::Shadow : SpectrumPlus2a::ScreenBuffer::Normal;
    spectrum().setScreenBuffer(screenBuffer);

    if (value & DisablePagingMask) {
        setPagingEnabled(false);
    }
}

void SpectrumPlus2aPagingDevice::writePort1ffd(::Z80::UnsignedByte value)
{
    auto * memory = dynamic_cast<SpectrumPlus2aMemory *>(spectrum().memory());
    assert(memory);

    if (value & PagingModeMask) {
        // special paging mode
        memory->setSpecialPagingConfiguration(static_cast<SpecialPagingConfiguration>((value & SpecialPagingConfigurationMask) >> SpecialPagingConfigurationShift));
    } else {
        // rom number high bit is in bit 2; low bit is retained from current ROM
        auto rom = static_cast<SpectrumPlus2aMemory::RomNumber>(
                ((value & RomNumberHighMask) >> (RomNumberHighBit - 1))
                | (static_cast<std::uint8_t>(memory->currentRom()) & 0x01)
            );
        memory->pageRom(rom);
        
        if (value & DiskMotorMask) {
            // TODO set disk motor running
        } else {
            // TODO set disk motor idle
        }
        
        // TODO printer strobe ...
    }
}