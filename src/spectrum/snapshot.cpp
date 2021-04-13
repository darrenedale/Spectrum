//
// Created by darren on 14/03/2021.
//

#include "snapshot.h"
#include "displaydevice.h"
#include "spectrum48k.h"
#include "spectrum16k.h"
#include "spectrum128k.h"
#include "spectrumplus2.h"
#include "spectrumplus2a.h"

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
: m_model(Model::Spectrum48k),
  m_registers(std::move(registers)),
  m_memory(memory ? memory->clone() : nullptr),
  iff1(false),
  iff2(false),
  im(InterruptMode::IM0),
  border(Colour::White)
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
  border(other.border)
{}

Snapshot::Snapshot(Snapshot && other) noexcept
: m_model(other.m_model),
  m_registers(std::move(other.m_registers)),
  m_memory(std::move(other.m_memory)),
  iff1(other.iff1),
  iff2(other.iff2),
  im(other.im),
  border(other.border)
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

        case Model::Spectrum128k: {
                auto * memory = dynamic_cast<Spectrum128k::MemoryType *>(m_memory.get());
                auto * spectrumMemory = dynamic_cast<Spectrum128k::MemoryType *>(spectrum.memory());
                assert(memory);
                assert(spectrumMemory);

                for (std::uint8_t bankNumber = 0; bankNumber < 8; ++bankNumber) {
                    spectrumMemory->writeToBank(static_cast<MemoryBankNumber128k>(bankNumber), memory->bankPointer(static_cast<MemoryBankNumber128k>(bankNumber)), Spectrum128KMemory::BankSize);
                }

                if (std::holds_alternative<RomNumber128k>(romNumber)) {
                    spectrumMemory->pageRom(std::get<RomNumber128k>(romNumber));
                } else {
                    spectrumMemory->pageRom(RomNumber128k::Rom0);
                }

                spectrumMemory->pageBank(pagedBankNumber);
            }
            break;

        case Model::SpectrumPlus2: {
                auto * memory = dynamic_cast<SpectrumPlus2::MemoryType *>(m_memory.get());
                auto * spectrumMemory = dynamic_cast<SpectrumPlus2::MemoryType *>(spectrum.memory());
                assert(memory);
                assert(spectrumMemory);
                spectrumMemory->pageBank(pagedBankNumber);

                if (std::holds_alternative<RomNumber128k>(romNumber)) {
                    spectrumMemory->pageRom(std::get<RomNumber128k>(romNumber));
                } else {
                    spectrumMemory->pageRom(RomNumber128k::Rom0);
                }

                for (std::uint8_t bankNumber = 0; bankNumber < 8; ++bankNumber) {
                    spectrumMemory->writeToBank(static_cast<MemoryBankNumber128k>(bankNumber), memory->bankPointer(static_cast<MemoryBankNumber128k>(bankNumber)), Spectrum128KMemory::BankSize);
                }
            }
            break;

        case Model::SpectrumPlus2a:
            assert(dynamic_cast<Spectrum16k::MemoryType *>(m_memory.get()));
            std::cerr << "Snapshots for model type " << std::to_string(model()) << " not yet implemented\n";
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
