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
        SpectrumPlus3,
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
     * Paging modes for the +2a/+3.
     */
    enum class PagingMode : std::uint8_t
    {
        Normal,
        Special,
    };

    /**
     * Special paging configurations for the +2a/+3
     */
    enum class SpecialPagingConfiguration : std::uint8_t
    {
        Config1 = 0,
        Config2 = 1,
        Config3 = 2,
        Config4 = 3,
    };

    /**
     * Write a human-readable representation of a Spectrum colour to an output stream.
     *
     * @param out The stream to write to.
     * @param colour The colour to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, Colour colour);

    /**
     * Write a human-readable representation of a Spectrum model to an output stream.
     *
     * @param out The stream to write to.
     * @param model The model type to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, Model model);

    /**
     * Write a human-readable representation of a Spectrum 128K ROM number.
     *
     * @param out The stream to write to.
     * @param rom The rom to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, RomNumber128k rom);

    /**
     * Write a human-readable representation of a Spectrum +2a/+3 ROM number.
     *
     * @param out The stream to write to.
     * @param rom The rom to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, RomNumberPlus2a rom);

    /**
     * Write a human-readable representation of a Spectrum screen buffer type to an output stream.
     *
     * @param out The stream to write to.
     * @param bufferType The buffer type to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, ScreenBuffer128k bufferType);

    /**
     * Write a human-readable representation of a Spectrum +2a/+3 paging mode to an output stream.
     *
     * @param out The stream to write to.
     * @param mode The mode to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, PagingMode mode);

    /**
     * Write a human-readable representation of a Spectrum special paging configuration to an output stream.
     *
     * @param out The stream to write to.
     * @param config The configuration to write.
     *
     * @return
     */
    std::ostream & operator<<(std::ostream & out, SpecialPagingConfiguration config);
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
     * Fetch a human-readable string representation of a Spectrum 128K ROM.
     */
    std::string to_string(Spectrum::RomNumber128k rom);

    /**
     * Fetch a human-readable string representation of a Spectrum +2a/+3 ROM.
     */
    std::string to_string(Spectrum::RomNumberPlus2a rom);

    /**
     * Fetch a human-readable string representation of a Spectrum screen buffer type.
     */
    std::string to_string(Spectrum::ScreenBuffer128k);

    /**
     * Fetch a human-readable string representation of a Spectrum +2a/+3 paging mode.
     */
    std::string to_string(Spectrum::PagingMode);

    /**
     * Fetch a human-readable string representation of a Spectrum +2a/+3 special paging mode configuration.
     */
    std::string to_string(Spectrum::SpecialPagingConfiguration);
}

#endif //SPECTRUM_TYPES_H
