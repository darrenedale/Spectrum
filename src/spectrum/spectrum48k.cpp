#include <fstream>
#include <print>

#include "snapshot.h"
#include "spectrum48k.h"
#include "devices/displaydevice.h"
#include "../util/assert.h"

using namespace Spectrum;

Spectrum48k::Spectrum48k(const std::string & romFile)
: BaseSpectrum(std::make_unique<Spectrum::Memory::SimpleSpectrumMemory>())
{
    loadRom(romFile);
}

Spectrum48k::Spectrum48k()
: Spectrum48k(std::string{})
{}

Spectrum48k::~Spectrum48k() = default;

bool Spectrum48k::loadRom(const std::string & fileName)
{
    static constexpr std::size_t RomFileSize = 0x4000;
    std::ifstream inFile(fileName, std::ios::binary | std::ios::in);

    if (!inFile) {
        std::cerr << "spectrum ROM file \"" << fileName << "\" could not be opened.\n";
        return false;
    }

    inFile.read(reinterpret_cast<std::ifstream::char_type *>(memory()->pointerTo(0)), RomFileSize);

    if (inFile.fail()) {
        std::cerr << "failed to load spectrum ROM file \"" << fileName << "\".\n";
        return false;
    }

    m_romFile = fileName;
    return true;
}

void Spectrum48k::reloadRoms()
{
    loadRom(m_romFile);
}

std::unique_ptr<Snapshot> Spectrum48k::snapshot() const
{
    return std::make_unique<Snapshot>(*this);
}

bool Spectrum48k::canApplySnapshot(const Snapshot & snapshot)
{
    return snapshot.model() == model() && dynamic_cast<const MemoryType *>(snapshot.memory());
}

void Spectrum48k::applySnapshot(const Snapshot & snapshot)
{
    sp_assert(snapshot.model() == model(), "detected Spectrum model mismatch in Spectrum::Spectrum48k::applySnapshot()");
    sp_assert(dynamic_cast<const MemoryType *>(snapshot.memory()), "found incompatible snapshot memory in Spectrum::Spectrum48k::applySnapshot()");

    reset();
    applySnapshotCpuState(snapshot);

    for (auto * display : displayDevices()) {
        display->setBorder(snapshot.border, false);
    }

    memory()->writeBytes(0x4000, 0x10000 - 0x4000, snapshot.memory()->pointerTo(0x4000));
}
