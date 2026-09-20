#include <iostream>
#include <iomanip>
#include <fstream>
#include "spectrum16k.h"
#include "devices/displaydevice.h"
#include "snapshot.h"

using namespace Spectrum;

Spectrum16k::Spectrum16k(const std::string & romFile)
: BaseSpectrum(std::make_unique<Memory::SimpleSpectrumMemory>(0x8000))
{
    loadRom(romFile);
}

Spectrum16k::Spectrum16k()
: Spectrum16k(std::string{})
{}

Spectrum16k::~Spectrum16k() = default;

bool Spectrum16k::loadRom(const std::string & romFilePath)
{
    static constexpr const std::size_t RomFileSize = 0x4000;
    std::ifstream inFile(romFilePath, std::ios::binary | std::ios::in);

    if (!inFile) {
        std::cerr << "spectrum ROM file \"" << romFilePath << "\" could not be opened.\n";
        return false;
    }

    inFile.read(reinterpret_cast<std::ifstream::char_type *>(memory()->pointerTo(0)), RomFileSize);

    if (inFile.fail()) {
        std::cerr << "failed to load spectrum ROM file \"" << romFilePath << "\".\n";
        return false;
    }

    m_romFile = romFilePath;
    return true;
}

void Spectrum16k::reloadRoms()
{
    loadRom(m_romFile);
}

std::unique_ptr<Snapshot> Spectrum16k::snapshot() const
{
    return std::make_unique<Snapshot>(*this);
}

bool Spectrum16k::canApplySnapshot(const Snapshot & snapshot)
{
    return snapshot.model() == model() && dynamic_cast<const MemoryType *>(snapshot.memory());
}

void Spectrum16k::applySnapshot(const Snapshot & snapshot)
{
    assert(snapshot.model() == model());
    assert(dynamic_cast<const MemoryType *>(snapshot.memory()));

    reset();
    applySnapshotCpuState(snapshot);

    for (auto * display : displayDevices()) {
        display->setBorder(snapshot.border, false);
    }

    memory()->writeBytes(0x4000, 0x8000 - 0x4000, snapshot.memory()->pointerTo(0x4000));
}
