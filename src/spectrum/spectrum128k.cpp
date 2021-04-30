#include <cassert>
#include <fstream>
#include "spectrum128k.h"
#include "memory128k.h"
#include "basespectrum.h"
#include "displaydevice.h"
#include "snapshot.h"

using namespace Spectrum;

using ::Z80::UnsignedByte;

Spectrum128k::Spectrum128k(const std::string & romFile0, const std::string & romFile1)
: BaseSpectrum(std::make_unique<Memory128k>()),
  m_pager(*this),
  m_screenBuffer(ScreenBuffer::Normal),
  m_romFiles{romFile0, romFile1}
{
    auto * mem = memory128();
    mem->loadRom(romFile0, 0);
    mem->loadRom(romFile1, 1);
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
        return memory128()->pagePointer(7);
    }

    return memory128()->pagePointer(5);
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
    memory128()->pageRom(0);
    memory128()->pageRam(0);
}

void Spectrum128k::reloadRoms()
{
    assert(memory128());
    memory128()->loadRom(m_romFiles[0], 0);
    memory128()->loadRom(m_romFiles[1], 1);
}

std::unique_ptr<Snapshot> Spectrum128k::snapshot() const
{
    auto snapshot = std::make_unique<Snapshot>(*this);
    snapshot->pagedBankNumber = memory128()->currentRamPage();
    snapshot->romNumber = memory128()->currentRom();
    snapshot->screenBuffer = screenBuffer();
    snapshot->pagingEnabled = pager()->pagingEnabled();
    return snapshot;
}

bool Spectrum128k::canApplySnapshot(const Snapshot & snapshot)
{
    return snapshot.model() == model()
        && dynamic_cast<const MemoryType *>(snapshot.memory())
        && 2 > snapshot.romNumber;
}

void Spectrum128k::applySnapshot(const Snapshot & snapshot)
{
    assert(snapshot.model() == model());
    assert(2 > snapshot.romNumber);
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
    memory->pageRom(snapshot.romNumber);
    memory->pageRam(snapshot.pagedBankNumber);

    for (int page = 0; page < 8; ++page) {
        memory->writeToPage(page, snapshotMemory->pagePointer(page), Memory128k::PageSize);
    }
}
