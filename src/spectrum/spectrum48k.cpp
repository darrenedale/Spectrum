#include "spectrum48k.h"

using namespace Spectrum;

Spectrum48k::Spectrum48k(const std::string & romFile)
: Spectrum48k()
{
    (void) loadRom(romFile);
}

Spectrum48k::Spectrum48k()
: BaseSpectrum(0x10000)
{}

Spectrum48k::~Spectrum48k() = default;