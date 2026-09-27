#include <fstream>
#include <print>

#include "basespectrum.h"
#include "snapshot.h"
#include "spectrum128k.h"
#include "devices/displaydevice.h"
#include "memory/memory128k.h"
#include "../util/assert.h"

using namespace Spectrum;

using ::Z80::UnsignedByte;

Spectrum128k::Spectrum128k(const std::string & romFile0, const std::string & romFile1)
: BaseSpectrum(std::make_unique<Memory::Memory128k>()),
  m_pager(*this),
  m_screenBuffer(ScreenBuffer::Normal),
  m_romFiles{romFile0, romFile1}
{
    auto * mem = memory128();
    mem->loadRom(romFile0, 0);
    mem->loadRom(romFile1, 1);
    auto * cpu = z80();
    sp_assert(cpu, "null Cpu provided to Spectrum128k constructor");
    cpu->connectIODevice(&m_pager);
}

Spectrum128k::Spectrum128k()
: Spectrum128k({}, {})
{}

DisplayFile Spectrum128k::displayMemory() const
{
    sp_assert(memory128(), "Spectrum has no memory in Spectrum128k::displayMemory()");

    if (ScreenBuffer::Shadow == m_screenBuffer) {
        return DisplayFile(memory128()->pagePointer(7), DisplayFile::extent);
    }

    return DisplayFile(memory128()->pagePointer(5), DisplayFile::extent);
}

Spectrum128k::~Spectrum128k()
{
    if (auto * cpu = z80()) {
        cpu->disconnectIODevice(&m_pager);
    }
}

void Spectrum128k::reset()
{
    sp_assert(memory128(), "Spectrum has no memory in Spectrum128k::reset()");

    // NOTE base class method triggers reload of ROM images
    BaseSpectrum::reset();
    m_screenBuffer = ScreenBuffer::Normal;
    m_pager.reset();
    memory128()->pageRom(0);
    memory128()->pageRam(0);
}

void Spectrum128k::reloadRoms()
{
    sp_assert(memory128(), "Spectrum has no memory in Spectrum128k::reloadRoms()");

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
    sp_assert(snapshot.model() == model(), "Spectrum128k::applySnapshot called with a snapshot for a different model");
    sp_assert(2 > snapshot.romNumber, "snapshot has invalid ROM number {}", snapshot.romNumber);
    auto * snapshotMemory = dynamic_cast<const MemoryType *>(snapshot.memory());
    sp_assert(snapshotMemory, "snapshot has no memory in Spectrum128k::applySnapshot()");

    reset();
    applySnapshotCpuState(snapshot);

    for (auto * display : displayDevices()) {
        display->setBorder(snapshot.border, false);
    }

    setScreenBuffer(snapshot.screenBuffer);
    pager()->setPagingEnabled(snapshot.pagingEnabled);
    auto * memory = memory128();
    memory->pageRom(snapshot.romNumber);
    memory->pageRam(snapshot.pagedBankNumber);

    for (int page = 0; page < 8; ++page) {
        memory->writeToPage(page, snapshotMemory->pagePointer(page), Memory::Memory128k::PageSize, {});
    }
}
