#include "spectrum16k.h"

using namespace Spectrum;
Spectrum16k::Spectrum16k(const std::string & romFile)
        : Spectrum16k()
{
    (void) loadRom(romFile);
}

Spectrum16k::Spectrum16k()
        : BaseSpectrum(0x7fff, nullptr)
{}

Spectrum16k::~Spectrum16k() = default;
