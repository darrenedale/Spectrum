//
// Created by darren on 15/03/2021.
//

#ifndef SPECTRUM_TYPES_H
#define SPECTRUM_TYPES_H

#include <cstdint>
#include <string>

namespace Spectrum
{
    enum class Colour : std::uint8_t
    {
        Black = 0,
        Blue,
        Red,
        Magenta,
        Green,
        Cyan,
        Yellow,
        White,
    };

    enum class Model : std::uint8_t
    {
        Spectrum16k,
        Spectrum48k,
        Spectrum128k,
        SpectrumPlus2,
        SpectrumPlus2a,
    };

    /**
     * Enumeration of the available RAM banks for 128k models.
     */
    enum class MemoryBankNumber128k : std::uint8_t
    {
        Bank0 = 0,
        Bank1,
        Bank2,
        Bank3,
        Bank4,
        Bank5,
        Bank6,
        Bank7,
    };

    /**
     * Available ROMs for 128k/plus2
     */
    enum class RomNumber128k : std::uint8_t
    {
        Rom0 = 0,
        Rom1,
    };

    /**
     * Available ROMs for plus2a/plus3
     */
    enum class RomNumberPlus2a : std::uint8_t
    {
        Rom0 = 0,
        Rom1,
        Rom2,
        Rom3,
    };

}

namespace std
{
    std::string to_string(Spectrum::Model model);
}

#endif //SPECTRUM_TYPES_H
