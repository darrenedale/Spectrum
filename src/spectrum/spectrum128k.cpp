#include <fstream>
#include <cassert>
#include "spectrum128k.h"
#include "spectrum128kmemory.h"
#include "basespectrum.h"

using namespace Spectrum;

using ::Z80::UnsignedByte;
using RomNumber = Spectrum128KMemory::RomNumber;
using BankNumber = Spectrum128KMemory::BankNumber;

Spectrum128k::Spectrum128k(const std::string & romFile0, const std::string & romFile1)
: BaseSpectrum(new Spectrum128KMemory()),
  m_pager(*this),
  m_screenBuffer(ScreenBuffer::Normal),
  m_romFiles{romFile0, romFile1}
{
    auto * mem = memory128();
    mem->loadRom(romFile0, RomNumber::Rom0);
    mem->loadRom(romFile1, RomNumber::Rom1);
    auto * cpu = z80();
    assert(cpu);
    cpu->connectIODevice(&m_pager);
}

Spectrum128k::Spectrum128k()
: Spectrum128k(std::string{}, std::string{})
{}

UnsignedByte * Spectrum128k::displayMemory() const
{
    assert(memory128());

    if (ScreenBuffer::Shadow == m_screenBuffer) {
        return memory128()->bankPointer(BankNumber::Bank7);
    }

    return memory128()->bankPointer(BankNumber::Bank5);
}

Spectrum128k::~Spectrum128k()
{
    auto * cpu = z80();

    if (cpu) {
        cpu->disconnectIODevice(&m_pager);
    }
}

void Spectrum128k::reset()
{
    assert(memory128());
    BaseSpectrum::reset();
    m_screenBuffer = ScreenBuffer::Normal;
    m_pager.reset();
    memory128()->pageRom(RomNumber::Rom0);
    memory128()->pageBank(BankNumber::Bank0);
}

void Spectrum128k::reloadRoms()
{
    auto * mem = memory128();
    assert(mem);
    mem->loadRom(m_romFiles[0], RomNumber::Rom0);
    mem->loadRom(m_romFiles[1], RomNumber::Rom1);
}
