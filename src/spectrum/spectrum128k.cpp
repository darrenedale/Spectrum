#include <cassert>
#include <fstream>
#include "spectrum128k.h"
#include "spectrum128kmemory.h"
#include "basespectrum.h"
#include "displaydevice.h"
#include "snapshot.h"

using namespace Spectrum;

using ::Z80::UnsignedByte;
using RomNumber = Spectrum128KMemory::RomNumber;
using BankNumber = Spectrum128KMemory::BankNumber;

// NOTE the base class constructor ensures that the Computer instance owns the allocated memory object and will destroy
// it in its destructor
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
: Spectrum128k({}, {})
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
    // NOTE base class method triggers reload of ROM images
    BaseSpectrum::reset();
    m_screenBuffer = ScreenBuffer::Normal;
    m_pager.reset();
    memory128()->pageRom(RomNumber::Rom0);
    memory128()->pageBank(BankNumber::Bank0);
}

void Spectrum128k::reloadRoms()
{
    assert(memory128());
    memory128()->loadRom(m_romFiles[0], RomNumber::Rom0);
    memory128()->loadRom(m_romFiles[1], RomNumber::Rom1);
}

std::unique_ptr<Snapshot> Spectrum128k::snapshot() const
{
    auto snapshot = std::make_unique<Snapshot>(*this);
    snapshot->pagedBankNumber = memory128()->currentPagedBank();
    snapshot->romNumber = memory128()->currentRom();
    snapshot->screenBuffer = screenBuffer();
    snapshot->pagingEnabled = pager()->pagingEnabled();
    return snapshot;
}

bool Spectrum128k::canApplySnapshot(const Snapshot & snapshot)
{
    return snapshot.model() == model()
        && dynamic_cast<const MemoryType *>(snapshot.memory())
        && std::holds_alternative<RomNumber128k>(snapshot.romNumber);
}

void Spectrum128k::applySnapshot(const Snapshot & snapshot)
{
    assert(snapshot.model() == model());
    assert(std::holds_alternative<RomNumber128k>(snapshot.romNumber));
    auto * snapshotMemory = dynamic_cast<const MemoryType *>(snapshot.memory());
    assert(snapshotMemory);

    reset();
    applySnapshotCpuState(snapshot);

    for (auto * display : displayDevices()) {
        display->setBorder(snapshot.border);
    }

    setScreenBuffer(snapshot.screenBuffer);
    pager()->setPagingEnabled(snapshot.pagingEnabled);
    auto * memory = memory128();
    memory->pageRom(std::get<RomNumber128k>(snapshot.romNumber));
    memory->pageBank(snapshot.pagedBankNumber);

    for (std::uint8_t bankNumber = 0; bankNumber < 8; ++bankNumber) {
        memory->writeToBank(static_cast<MemoryBankNumber128k>(bankNumber),
                                    snapshotMemory->bankPointer(static_cast<MemoryBankNumber128k>(bankNumber)),
                                    MemoryType::BankSize);
    }
}
