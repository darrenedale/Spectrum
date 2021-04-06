#include <fstream>

#include <iostream>
#include <iomanip>
#include <cstring>
#include "spectrum128k.h"

using namespace Spectrum;

using ::Z80::UnsignedByte;

namespace
{
    constexpr const int RomSize = 0x4000;
    constexpr const int MemoryBankSize = 0x4000;

    constexpr const int MemoryBankOffset[8] = {
        0x10000,
        0x14000,
        0x8000,     // NOTE bank 2 is always at 0x8000, and can also be paged into 0xc000 so we use the memory at 0x8000 as the canonical data
        0x18000,
        0x1c000,
        0x4000,     // NOTE bank 2 is always at 0x4000, and can also be paged into 0xc000 so we use the memory at 0x8000 as the canonical data
        0x20000,
        0x24000,
    };

    constexpr const int NormalDisplayMemoryOffset = 0x4000;
    constexpr const int ShadowDisplayMemoryOffset = MemoryBankOffset[7];
}

Spectrum128k::Spectrum128k(const std::string & romFile128, const std::string & romFile48)
: Spectrum128k()
{
    {
        std::ifstream in(romFile128);

        if (!in) {
            std::cerr << "failed to open 128k ROM image file\n";
            // TODO throw
        }

        in.read(reinterpret_cast<std::ifstream::char_type *>(m_romImages.rom[0]), RomSize);

        if (in.fail() && !in.eof()) {
            std::cerr << "failed to read 128k ROM image\n";
            // TODO throw
        }
    }

    {
        std::ifstream in(romFile48);

        if (!in) {
            std::cerr << "failed to open 48k ROM image file\n";
            // TODO throw
        }

        in.read(reinterpret_cast<std::ifstream::char_type *>(m_romImages.rom[1]), RomSize);

        if (in.fail() && !in.eof()) {
            std::cerr << "failed to read 48k ROM image\n";
            // TODO throw
        }
    }

    std::memcpy(memory(), m_romImages.rom[0], RomSize);
}

Spectrum128k::Spectrum128k()
: BaseSpectrum(0x28000, nullptr),
  m_romImages(),
  m_currentRomNumber(Rom::Rom0),
  m_pagedMemoryBank(MemoryBank::Bank0)
{}

UnsignedByte * Spectrum128k::displayMemory() const
{
    if (ScreenBuffer::Shadow == m_screenBuffer) {
        if (MemoryBank::Bank7 == pagedMemoryBank()) {
            // while paged-in the memory may be different from the cache so we must read the shadow display buffer from
            // the paged-in location rather than the cached location while it's paged in otherwise the screen won't be up-
            // to-date
            return memory() + MemoryBankOffset[7];
        }

        return memory() + ShadowDisplayMemoryOffset;
    }

    return memory() + NormalDisplayMemoryOffset;
}

void Spectrum128k::pageMemoryBank(Spectrum128k::MemoryBank bank)
{
    if (bank == pagedMemoryBank()) {
        return;
    }

    // page out
    std::memcpy(pagedMemoryCache(), pagedMemory(), MemoryBankSize);
    m_pagedMemoryBank = bank;

    // page in
    std::memcpy(pagedMemory(), pagedMemoryCache(), MemoryBankSize);
}

::Z80::UnsignedByte *Spectrum128k::pagedMemory() const
{
    return memory() + 0xc000;
}

::Z80::UnsignedByte *Spectrum128k::pagedMemoryCache() const
{
    return memory() + MemoryBankOffset[static_cast<int>(pagedMemoryBank())];
}

void Spectrum128k::setRom(Spectrum128k::Rom rom)
{
    if (rom == m_currentRomNumber) {
        return;
    }

    std::memcpy(memory(), m_romImages.rom[static_cast<int>(rom)], RomSize);
}

Spectrum128k::~Spectrum128k() = default;
