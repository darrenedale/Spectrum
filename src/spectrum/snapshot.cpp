//
// Created by darren on 14/03/2021.
//

#include <iomanip>
#include "snapshot.h"
#include "displaydevice.h"
#include "spectrum48k.h"

using namespace Spectrum;
using InterruptMode = ::Z80::InterruptMode;

Snapshot::Snapshot(Model model)
: Snapshot({}, nullptr)
{
    m_model = model;
}

Snapshot::Snapshot(const BaseSpectrum & spectrum)
: Snapshot(spectrum.z80()->registers(), spectrum.memory())
{
    m_model = spectrum.model();
    auto * cpu = spectrum.z80();
    iff1 = cpu->iff1();
    iff2 = cpu->iff2();
    im = cpu->interruptMode();

    if (!spectrum.displayDevices().empty()) {
        border = spectrum.displayDevices().front()->border();
    }
}

Snapshot::Snapshot(Registers registers, BaseSpectrum::MemoryType * memory)
: m_model(),        // NOTE private constructor, model is always set in the other constructors that delegate to this
  m_registers(std::move(registers)),
  m_memory(memory ? memory->clone() : nullptr),
  iff1(false),
  iff2(false),
  im(InterruptMode::IM0),
  border(Colour::White),
  pagedBankNumber(MemoryBankNumber128k::Bank0),
  romNumber(RomNumber128k::Rom0),
  screenBuffer(ScreenBuffer128k::Normal),
  pagingEnabled(true),
  pagingMode(PagingMode::Normal),
  specialPagingConfig()
{}

Snapshot::~Snapshot() noexcept = default;

Snapshot::Snapshot(const Snapshot & other)
: m_model(other.m_model),
  m_registers(other.m_registers),
  m_memory(other.m_memory ? other.m_memory->clone() : nullptr),
  iff1(other.iff1),
  iff2(other.iff2),
  im(other.im),
  border(other.border),
  pagedBankNumber(other.pagedBankNumber),
  romNumber(other.romNumber),
  screenBuffer(ScreenBuffer128k::Normal),
  pagingEnabled(other.pagingEnabled),
  pagingMode(PagingMode::Normal),
  specialPagingConfig()
{}

Snapshot::Snapshot(Snapshot && other) noexcept
: m_model(other.m_model),
  m_registers(std::move(other.m_registers)),
  m_memory(std::move(other.m_memory)),
  iff1(other.iff1),
  iff2(other.iff2),
  im(other.im),
  border(other.border),
  pagedBankNumber(other.pagedBankNumber),
  romNumber(other.romNumber),
  screenBuffer(ScreenBuffer128k::Normal),
  pagingEnabled(other.pagingEnabled),
  pagingMode(other.pagingMode),
  specialPagingConfig(other.specialPagingConfig)
{}

Snapshot & Snapshot::operator=(const Snapshot & other)
{
    if (this == &other) {
        return *this;
    }

    m_model = other.m_model;
    m_registers = other.m_registers;
    iff1 = other.iff1;
    iff2 = other.iff2;
    im = other.im;
    border = other.border;
    pagedBankNumber = other.pagedBankNumber;
    romNumber = other.romNumber;
    screenBuffer = other.screenBuffer;
    pagingEnabled = other.pagingEnabled;
    pagingMode = other.pagingMode;
    specialPagingConfig = other.specialPagingConfig;

    if (other.m_memory) {
        m_memory = other.m_memory->clone();
    } else {
        m_memory.reset();
    }

    return *this;
}

Snapshot & Snapshot::operator=(Snapshot && other) noexcept
{
    if (this == &other) {
        return *this;
    }

    m_model = other.m_model;
    m_registers = other.m_registers;
    iff1 = other.iff1;
    iff2 = other.iff2;
    im = other.im;
    border = other.border;
    pagedBankNumber = other.pagedBankNumber;
    romNumber = other.romNumber;
    screenBuffer = other.screenBuffer;
    pagingEnabled = other.pagingEnabled;
    pagingMode = other.pagingMode;
    specialPagingConfig = other.specialPagingConfig;
    m_memory = std::move(other.m_memory);
    return *this;
}

#if (!defined(NDEBUG))
#include <iostream>
#include "../util/crc32.h"
#include "spectrum16k.h"
#include "spectrum48k.h"
#include "spectrum128k.h"
#include "spectrumplus2.h"
#include "spectrumplus2a.h"

std::ostream & Spectrum::operator<<(std::ostream & out, const Snapshot & snap)
{
    out << "Snapshot\n--------\n"
        << "  Model: " << std::to_string(snap.model()) << '\n'
        << "  Registers:\n"
        << std::hex << std::setfill('0')
        << "    AF   = 0x" << std::setw(4) << snap.registers().af << '\n'
        << "    BC   = 0x" << std::setw(4) << snap.registers().bc << '\n'
        << "    DE   = 0x" << std::setw(4) << snap.registers().de << '\n'
        << "    HL   = 0x" << std::setw(4) << snap.registers().hl << '\n'
        << "    A    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().a) << '\n'
        << "    F    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().f) << '\n'
        << "    B    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().b) << '\n'
        << "    C    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().c) << '\n'
        << "    D    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().d) << '\n'
        << "    E    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().e) << '\n'
        << "    H    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().h) << '\n'
        << "    L    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().l) << '\n'
        << "    I    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().i) << '\n'
        << "    R    = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().r) << '\n'
        << "    AF'  = 0x" << std::setw(4) << snap.registers().afShadow << '\n'
        << "    BC'  = 0x" << std::setw(4) << snap.registers().bcShadow << '\n'
        << "    DE'  = 0x" << std::setw(4) << snap.registers().deShadow << '\n'
        << "    HL'  = 0x" << std::setw(4) << snap.registers().hlShadow << '\n'
        << "    A'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().aShadow) << '\n'
        << "    F'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().fShadow) << '\n'
        << "    B'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().bShadow) << '\n'
        << "    C'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().cShadow) << '\n'
        << "    D'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().dShadow) << '\n'
        << "    E'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().eShadow) << '\n'
        << "    H'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().hShadow) << '\n'
        << "    L'   = 0x" << std::setw(2) << static_cast<std::uint16_t>(snap.registers().lShadow) << '\n'
        << "    PC   = 0x" << std::setw(4) << snap.registers().pc << '\n'
        << "    SP   = 0x" << std::setw(4) << snap.registers().sp << '\n'
        << "  Interrupts:\n"
        << "    IM   = " << std::to_string(snap.im) << '\n'
        << "    IFF1 = " << (snap.iff1 ? '1' : '0') << '\n'
        << "    IFF2 = " << (snap.iff2 ? '1' : '0') << '\n'
        << "  Display:\n"
        << "    Buffer: " << snap.screenBuffer << '\n'
        << "    Border: " << snap.border << '\n'
        << "  Memory:\n";

    if (Model::Spectrum48k == snap.model() && Model::Spectrum16k != snap.model()) {
        auto * memory = dynamic_cast<const Spectrum48k::MemoryType *>(snap.memory());
        out << "  48K checksum: 0x" << std::setw(8) << Util::crc32(memory->pointerTo(0x4000), 0xc000) << '\n';
    } else if (Model::Spectrum16k == snap.model()) {
        auto * memory = dynamic_cast<const Spectrum16k::MemoryType *>(snap.memory());
        out << "  16K checksum: 0x" << std::setw(8) << Util::crc32(memory->pointerTo(0x4000), 0x4000) << '\n';
    } else {
        out << "  ROM: " << std::dec;

        if (Model::SpectrumPlus2a == snap.model()) {
            out << static_cast<std::uint16_t>(std::get<RomNumberPlus2a>(snap.romNumber));
        } else {
            out << static_cast<std::uint16_t>(std::get<RomNumber128k>(snap.romNumber));
        }

        out << '\n'
            << "  Paging enabled: " << (snap.pagingEnabled ? "yes" : "no") << '\n'
            << "  Current page: " << static_cast<std::uint16_t>(snap.pagedBankNumber) << '\n';

        if (Model::SpectrumPlus2a == snap.model()) {
            out << "  Paging mode: " << snap.pagingMode << '\n';

            if (PagingMode::Special == snap.pagingMode) {
                out << "  Special paging mode config: " << snap.specialPagingConfig << '\n';
            }

            auto * memory = dynamic_cast<const SpectrumPlus2a::MemoryType *>(snap.memory());

            for (std::uint8_t page = 0; page < 8; ++page) {
                out << "  Page " << std::dec << static_cast<std::uint16_t>(page) << " checksum: 0x" << std::setw(8)
                    << Util::crc32(memory->bankPointer(static_cast<MemoryBankNumber128k>(page)), SpectrumPlus2a::MemoryType::BankSize) << '\n';
            }

        } else {
            auto * memory = dynamic_cast<const Spectrum128KMemory *>(snap.memory());

            for (std::uint8_t page = 0; page < 8; ++page) {
                out << "  Page " << std::dec << static_cast<std::uint16_t>(page) << " checksum: 0x" << std::setw(8)
                    << Util::crc32(memory->bankPointer(static_cast<MemoryBankNumber128k>(page)), SpectrumPlus2a::MemoryType::BankSize) << '\n';
            }
        }
    }

    out << std::dec << std::setfill(' ');
    return out;
}
#endif
