#include <cassert>
#include <fstream>
#include "spectrumplus2a.h"
#include "memoryplus2a.h"
#include "basespectrum.h"
#include "displaydevice.h"
#include "snapshot.h"

using namespace Spectrum;

using ::Z80::UnsignedByte;

SpectrumPlus2a::SpectrumPlus2a(const std::string & romFile0, const std::string & romFile1, const std::string & romFile2, const std::string & romFile3)
: BaseSpectrum(std::make_unique<MemoryPlus2a>()),
  m_pager(*this),
  m_screenBuffer(ScreenBuffer::Normal),
  m_romFiles{romFile0, romFile1, romFile2, romFile3}
{
    auto * mem = memoryPlus2a();
    mem->loadRom(romFile0, 0);
    mem->loadRom(romFile1, 1);
    mem->loadRom(romFile2, 2);
    mem->loadRom(romFile3, 3);
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
        return memoryPlus2a()->pagePointer(7);
    }

    return memoryPlus2a()->pagePointer(5);
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
    memoryPlus2a()->pageRom(0);
    memoryPlus2a()->pageRam(0);
}

void SpectrumPlus2a::reloadRoms()
{
    assert(memoryPlus2a());
    memoryPlus2a()->loadRom(m_romFiles[0], 0);
    memoryPlus2a()->loadRom(m_romFiles[1], 1);
    memoryPlus2a()->loadRom(m_romFiles[2], 2);
    memoryPlus2a()->loadRom(m_romFiles[3], 3);
}

std::unique_ptr<Snapshot> SpectrumPlus2a::snapshot() const
{
    auto snapshot = std::make_unique<Snapshot>(*this);
    snapshot->pagedBankNumber = memoryPlus2a()->currentRamPage();
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
           && snapshot.romNumber < 4;
}

void SpectrumPlus2a::applySnapshot(const Snapshot & snapshot)
{
    assert(snapshot.model() == model());
    assert(4 > snapshot.romNumber);
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
    memory->pageRom(snapshot.romNumber);
    memory->pageRam(snapshot.pagedBankNumber);
    memory->setPagingMode(snapshot.pagingMode);
    memory->setSpecialPagingConfiguration(snapshot.specialPagingConfig);

    for (int page = 0; page < 8; ++page) {
        memory->writeToPage(page, snapshotMemory->pagePointer(page), MemoryPlus2a::PageSize);
    }
}
