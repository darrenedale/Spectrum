#include "spectrum16k.h"

using namespace Spectrum;

Spectrum16k::Spectrum16k(const std::string & romFile)
: Spectrum16k()
{
    (void) loadRom(romFile);
}

Spectrum16k::Spectrum16k()
: BaseSpectrum(0x8000)
{}

Spectrum16k::~Spectrum16k() = default;
