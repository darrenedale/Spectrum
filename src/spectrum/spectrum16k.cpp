#include <iostream>
#include <iomanip>
#include <fstream>
#include "spectrum16k.h"

using namespace Spectrum;

Spectrum16k::Spectrum16k(const std::string & romFile)
: BaseSpectrum(0x8000)
{
    (void) loadRom(romFile);
}

Spectrum16k::Spectrum16k()
: Spectrum16k(std::string{})
{}

Spectrum16k::~Spectrum16k() = default;

bool Spectrum16k::loadRom(const std::string & fileName)
{
    static constexpr const std::size_t RomFileSize = 0x4000;
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

void Spectrum16k::reloadRoms()
{
    loadRom(m_romFile);
}

