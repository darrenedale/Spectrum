#include <cassert>
#include <fstream>
#include "spectrumplus2a.h"
#include "spectrumplus2amemory.h"
#include "basespectrum.h"
#include "displaydevice.h"
#include "snapshot.h"

using namespace Spectrum;

using ::Z80::UnsignedByte;
using RomNumber = SpectrumPlus2aMemory::RomNumber;
using BankNumber = SpectrumPlus2aMemory::BankNumber;

// NOTE the base class constructor ensures that the Computer instance owns the allocated memory object and will destroy
// it in its destructor
SpectrumPlus2a::SpectrumPlus2a(const std::string & romFile0, const std::string & romFile1, const std::string & romFile2, const std::string & romFile3)
: BaseSpectrum(new SpectrumPlus2aMemory()),
  m_pager(*this),
  m_screenBuffer(ScreenBuffer::Normal),
  m_romFiles{romFile0, romFile1, romFile2, romFile3}
{
    auto * mem = memoryPlus2a();
    mem->loadRom(romFile0, RomNumber::Rom0);
    mem->loadRom(romFile1, RomNumber::Rom1);
    mem->loadRom(romFile2, RomNumber::Rom2);
    mem->loadRom(romFile3, RomNumber::Rom3);
    auto * cpu = z80();
    assert(cpu);
    cpu->connectIODevice(&m_pager);
}

SpectrumPlus2a::SpectrumPlus2a()
: SpectrumPlus2a({}, {}, {}, {})
{}

UnsignedByte * SpectrumPlus2a::displayMemory() const
{
    assert(memoryPlus2a());

    if (ScreenBuffer::Shadow == m_screenBuffer) {
        return memoryPlus2a()->bankPointer(BankNumber::Bank7);
    }

    return memoryPlus2a()->bankPointer(BankNumber::Bank5);
}

SpectrumPlus2a::~SpectrumPlus2a()
{
    auto * cpu = z80();

    if (cpu) {
        cpu->disconnectIODevice(&m_pager);
    }
}

void SpectrumPlus2a::reset()
{
    assert(memoryPlus2a());
    // NOTE base class method triggers reload of ROM images
    BaseSpectrum::reset();
    m_screenBuffer = ScreenBuffer::Normal;
    m_pager.reset();
    memoryPlus2a()->pageRom(RomNumber::Rom0);
    memoryPlus2a()->pageBank(BankNumber::Bank0);
}

void SpectrumPlus2a::reloadRoms()
{
    assert(memoryPlus2a());
    memoryPlus2a()->loadRom(m_romFiles[0], RomNumber::Rom0);
    memoryPlus2a()->loadRom(m_romFiles[1], RomNumber::Rom1);
    memoryPlus2a()->loadRom(m_romFiles[2], RomNumber::Rom2);
    memoryPlus2a()->loadRom(m_romFiles[3], RomNumber::Rom3);
}

std::unique_ptr<Snapshot> SpectrumPlus2a::snapshot() const
{
    auto snapshot = std::make_unique<Snapshot>(*this);
    snapshot->pagedBankNumber = memoryPlus2a()->currentPagedBank();
    snapshot->romNumber = memoryPlus2a()->currentRom();
    snapshot->screenBuffer = screenBuffer();
    snapshot->pagingEnabled = pager()->pagingEnabled();
    snapshot->pagingMode = memoryPlus2a()->pagingMode();
    snapshot->specialPagingConfig = memoryPlus2a()->specialPagingConfiguration();
    return snapshot;
}

bool SpectrumPlus2a::canApplySnapshot(const Snapshot & snapshot)
{
    return snapshot.model() == model()
           && dynamic_cast<const MemoryType *>(snapshot.memory())
           && std::holds_alternative<RomNumberPlus2a>(snapshot.romNumber);
}

void SpectrumPlus2a::applySnapshot(const Snapshot & snapshot)
{
    assert(snapshot.model() == model());
    assert(std::holds_alternative<RomNumberPlus2a>(snapshot.romNumber));
    auto * snapshotMemory = dynamic_cast<const MemoryType *>(snapshot.memory());
    assert(snapshotMemory);

    reset();
    applySnapshotCpuState(snapshot);

    for (auto * display : displayDevices()) {
        display->setBorder(snapshot.border);
    }

    setScreenBuffer(snapshot.screenBuffer);
    pager()->setPagingEnabled(snapshot.pagingEnabled);
    auto * memory = memoryPlus2a();
    memory->pageRom(std::get<RomNumberPlus2a>(snapshot.romNumber));
    memory->pageBank(snapshot.pagedBankNumber);
    memory->setPagingMode(snapshot.pagingMode);
    memory->setSpecialPagingConfiguration(snapshot.specialPagingConfig);

    for (std::uint8_t bankNumber = 0; bankNumber < 8; ++bankNumber) {
        memory->writeToBank(static_cast<MemoryBankNumber128k>(bankNumber),
                            snapshotMemory->bankPointer(static_cast<MemoryBankNumber128k>(bankNumber)),
                            MemoryType::BankSize);
    }
}
