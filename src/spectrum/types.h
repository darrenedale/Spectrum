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
        SpectrumPlus2a,
    };

}
namespace std
{
    std::string to_string(Spectrum::Model model);
}

#endif //SPECTRUM_TYPES_H
