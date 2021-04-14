//
// Created by darren on 14/03/2021.
//

#include <iostream>
#include <iomanip>
#include "snapshot.h"
#include "displaydevice.h"
#include "spectrum48k.h"
#include "spectrum16k.h"
#include "spectrum128k.h"
#include "spectrumplus2.h"
#include "spectrumplus2a.h"
#include "../util/crc32.h"

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

    {
        auto * spectrum128 = dynamic_cast<const Spectrum128k *>(&spectrum);

        if (spectrum128) {
            auto * memory = dynamic_cast<const Spectrum128k::MemoryType *>(spectrum128->memory());
            pagedBankNumber = memory->currentPagedBank();
            romNumber = memory->currentRom();
            screenBuffer = spectrum128->screenBuffer();
            pagingEnabled = spectrum128->pager()->pagingEnabled();
        }
    }

    {
        auto * spectrumPlus2a = dynamic_cast<const SpectrumPlus2a *>(&spectrum);

        if (spectrumPlus2a) {
            auto * memory = dynamic_cast<const Spectrum128k::MemoryType *>(spectrumPlus2a->memory());
            pagedBankNumber = memory->currentPagedBank();
            romNumber = memory->currentRom();
            screenBuffer = spectrumPlus2a->screenBuffer();
            pagingEnabled = spectrumPlus2a->pager()->pagingEnabled();
        }
    }
}

Snapshot::Snapshot(Registers registers, BaseSpectrum::MemoryType * memory)
: m_model(Model::Spectrum48k),
  m_registers(std::move(registers)),
  m_memory(memory ? memory->clone() : nullptr),
  iff1(false),
  iff2(false),
  im(InterruptMode::IM0),
  border(Colour::White),
  pagedBankNumber(MemoryBankNumber128k::Bank0),
  romNumber(RomNumber128k::Rom0),
  screenBuffer(ScreenBuffer128k::Normal),
  pagingEnabled(true)
{
}

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
  pagingEnabled(other.pagingEnabled)
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
  pagingEnabled(other.pagingEnabled)
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
    m_memory = std::move(other.m_memory);
    return *this;
}

void Snapshot::applyTo(BaseSpectrum & spectrum) const
{
    assert (spectrum.model() == model());
    assert(m_memory);

    spectrum.reset();
    auto * cpu = spectrum.z80();
    cpu->registers() = m_registers;
    cpu->setIff1(iff1);
    cpu->setIff2(iff2);
    cpu->setInterruptMode(im);

    for (auto * display : spectrum.displayDevices()) {
        display->setBorder(border);
    }

    // NOTE we can't simply replace the memory object in the Spectrum because the snapshot memory doesn't have the ROM
    // loaded
    // TODO consider what we could do about this
    switch (model()) {
        case Model::Spectrum48k:
            assert(dynamic_cast<Spectrum48k::MemoryType *>(m_memory.get()));
            spectrum.memory()->writeBytes(0x4000, 0x10000 - 0x4000, m_memory->pointerTo(0x4000));
            break;

        case Model::Spectrum16k:
            assert(dynamic_cast<Spectrum16k::MemoryType *>(m_memory.get()));
            spectrum.memory()->writeBytes(0x4000, 0x8000 - 0x4000, m_memory->pointerTo(0x4000));
            break;

            // TODO the cases for Spectrum 128K and Spectrum Plus2 can probably be merged
        case Model::Spectrum128k: {
                auto & spectrum128 = dynamic_cast<Spectrum128k &>(spectrum);
                auto * memory = dynamic_cast<Spectrum128k::MemoryType *>(m_memory.get());
                auto * spectrumMemory = dynamic_cast<Spectrum128k::MemoryType *>(spectrum128.memory());
                assert(memory);
                assert(spectrumMemory);
                spectrum128.setScreenBuffer(screenBuffer);
                spectrum128.pager()->setPagingEnabled(pagingEnabled);
                spectrumMemory->pageBank(pagedBankNumber);

                if (std::holds_alternative<RomNumber128k>(romNumber)) {
                    spectrumMemory->pageRom(std::get<RomNumber128k>(romNumber));
                } else {
                    spectrumMemory->pageRom(RomNumber128k::Rom0);
                }

                for (std::uint8_t bankNumber = 0; bankNumber < 8; ++bankNumber) {
                    spectrumMemory->writeToBank(static_cast<MemoryBankNumber128k>(bankNumber),
                                                memory->bankPointer(static_cast<MemoryBankNumber128k>(bankNumber)),
                                                Spectrum128KMemory::BankSize);
                }
            }
            break;

        case Model::SpectrumPlus2: {
                auto & spectrumPlus2 = dynamic_cast<SpectrumPlus2 &>(spectrum);
                auto * memory = dynamic_cast<SpectrumPlus2::MemoryType *>(m_memory.get());
                auto * spectrumMemory = dynamic_cast<SpectrumPlus2::MemoryType *>(spectrum.memory());
                assert(memory);
                assert(spectrumMemory);
                spectrumPlus2.setScreenBuffer(screenBuffer);
                spectrumPlus2.pager()->setPagingEnabled(pagingEnabled);
                spectrumMemory->pageBank(pagedBankNumber);

                if (std::holds_alternative<RomNumber128k>(romNumber)) {
                    spectrumMemory->pageRom(std::get<RomNumber128k>(romNumber));
                } else {
                    spectrumMemory->pageRom(RomNumber128k::Rom0);
                }

                for (std::uint8_t bankNumber = 0; bankNumber < 8; ++bankNumber) {
                    spectrumMemory->writeToBank(static_cast<MemoryBankNumber128k>(bankNumber),
                                                memory->bankPointer(static_cast<MemoryBankNumber128k>(bankNumber)),
                                                Spectrum128KMemory::BankSize);
                }
            }
            break;

        case Model::SpectrumPlus2a: {
                auto & spectrumPlus2a = dynamic_cast<SpectrumPlus2a &>(spectrum);
                auto * memory = dynamic_cast<SpectrumPlus2a::MemoryType *>(m_memory.get());
                auto * spectrumMemory = dynamic_cast<SpectrumPlus2a::MemoryType *>(spectrum.memory());
                assert(memory);
                assert(spectrumMemory);
                spectrumPlus2a.setScreenBuffer(screenBuffer);
                spectrumPlus2a.pager()->setPagingEnabled(pagingEnabled);
                spectrumMemory->pageBank(pagedBankNumber);

                if (std::holds_alternative<RomNumberPlus2a>(romNumber)) {
                    spectrumMemory->pageRom(std::get<RomNumberPlus2a>(romNumber));
                } else {
                    spectrumMemory->pageRom(RomNumberPlus2a::Rom0);
                }

                for (std::uint8_t bankNumber = 0; bankNumber < 8; ++bankNumber) {
                    spectrumMemory->writeToBank(static_cast<MemoryBankNumber128k>(bankNumber),
                                                memory->bankPointer(static_cast<MemoryBankNumber128k>(bankNumber)),
                                                Spectrum128KMemory::BankSize);
                }
            }
            break;
    }
}

void Snapshot::readFrom(BaseSpectrum & spectrum)
{
    auto * cpu = spectrum.z80();
    m_registers = cpu->registers();
    iff1 = cpu->iff1();
    iff2 = cpu->iff2();
    im = cpu->interruptMode();

    if (!spectrum.displayDevices().empty()) {
        border = spectrum.displayDevices().front()->border();
    } else {
        border = Colour::White;
    }

    m_memory = spectrum.memory()->clone();
}

bool Snapshot::canApplyTo(BaseSpectrum & spectrum) const
{
    if (spectrum.model() != model()) {
        std::cerr << "incompatible models\n";
        return false;
    }

    if(!(m_memory)) {
        return false;
    }

    switch (model()) {
        case Model::Spectrum48k:
            if (!dynamic_cast<Spectrum48k::MemoryType *>(m_memory.get())) {
                std::cerr << "incompatible memory types\n";
                return false;
            }
            break;

        case Model::Spectrum16k: // NOLINT(bugprone-branch-clone) 16K and 48K memory types happen to be the same ATM
            if (!dynamic_cast<Spectrum16k::MemoryType *>(m_memory.get())) {
                std::cerr << "incompatible memory types\n";
                return false;
            }
            break;

        case Model::Spectrum128k:
            if (!dynamic_cast<Spectrum128k::MemoryType *>(m_memory.get())) {
                std::cerr << "incompatible memory types\n";
                return false;
            }
            break;

        case Model::SpectrumPlus2:
            if (!dynamic_cast<SpectrumPlus2::MemoryType *>(m_memory.get())) {
                std::cerr << "incompatible memory types\n";
                return false;
            }
            break;

        case Model::SpectrumPlus2a:
            if (!dynamic_cast<SpectrumPlus2a::MemoryType *>(m_memory.get())) {
                std::cerr << "incompatible memory types\n";
                return false;
            }
            break;
    }

    return true;
}

#if (!defined(NDEBUG))
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
