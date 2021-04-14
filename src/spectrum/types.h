//
// Created by darren on 15/03/2021.
//

#ifndef SPECTRUM_TYPES_H
#define SPECTRUM_TYPES_H

#include <cstdint>
#include <string>

namespace Spectrum
{
    /**
     * Enumeration of Spectrum colours.
     */
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

    /**
     * Enumeration of the Spectrum models supported by the emulator.
     */
    enum class Model : std::uint8_t
    {
        Spectrum16k,
        Spectrum48k,
        Spectrum128k,
        SpectrumPlus2,
        SpectrumPlus2a,
    };

    /**
     * Enumeration of the available screen buffer types for 128k models.
     */
    enum class ScreenBuffer128k : std::uint8_t
    {
        Normal,       // display the normal screen buffer
        Shadow,       // display the shadow screen buffer
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

    /**
     * Write a human-readable representation of a Spectrum colour to an output stream.
     *
     * @param out The stream to write to.
     * @param colour The colour to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, Spectrum::Colour colour);

    /**
     * Write a human-readable representation of a Spectrum model to an output stream.
     *
     * @param out The stream to write to.
     * @param colour The model type to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, Spectrum::Model model);

    /**
     * Write a human-readable representation of a Spectrum screen buffer type to an output stream.
     *
     * @param out The stream to write to.
     * @param colour The buffer type to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, Spectrum::ScreenBuffer128k bufferType);

}

namespace std
{
    /**
     * Fetch a human-readable string representation of a Spectrum colour.
     */
    std::string to_string(Spectrum::Colour colour);

    /**
     * Fetch a human-readable string representation of a Spectrum model type.
     */
    std::string to_string(Spectrum::Model model);

    /**
     * Fetch a human-readable string representation of a Spectrum screen buffer type.
     */
    std::string to_string(Spectrum::ScreenBuffer128k);
}

#endif //SPECTRUM_TYPES_H
