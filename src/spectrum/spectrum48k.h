//
// Created by darren.
//

#ifndef SPECTRUM_SPECTRUM48K_H
#define SPECTRUM_SPECTRUM48K_H

#include <string>

#include "basespectrum.h"

namespace Spectrum
{
    /**
     * Abstraction of a Spectrum 48K
     *
     * The Spectrum 48K contains a Z80 running at roughly 3.5MHz and 64kb of addressable memory. The low 16kb is ROM,
     * the remainder is RAM. The memory model is a simple contiguous, linear array of 64436 bytes.
     *
     * The 6912 bytes of memory at 0x4000 (16384) - right at the beginning of the RAM segment of the address space - is
     * the display file, basically the Spectrum's VRAM.
     */
	class Spectrum48k
    : public BaseSpectrum
    {
    public:
        static constexpr const int DisplayMemoryOffset = 0x4000;
        static constexpr const int DisplayMemorySize = 6912;

        /**
         * Default constructor.
         *
         * Without a loaded ROM, the Spectrum is not likely to be much use.
         */
        Spectrum48k();

        /**
         * Initialise a new Spectrum48k with a ROM image on disk.
         *
         * @param romFile
         */
        explicit Spectrum48k(const std::string & romFile);

        // Spectrum48k's are not copy or move constructable or assignable
        Spectrum48k(const Spectrum48k &) = delete;
        Spectrum48k(Spectrum48k &&) = delete;
        void operator=(const Spectrum48k &) = delete;
        void operator=(Spectrum48k &&) = delete;

        ~Spectrum48k() override;

        /**
         * The Spectrum model type.
         *
         * Always Model::Spectrum48k.
         *
         * @return
         */
        [[nodiscard]] inline constexpr Model model() const override
        {
            return Model::Spectrum48k;
        }

        /**
         * Fetch a pointer to the Spectrum's display file.
         *
         * It's safe to use the returned pointer as a pointer to a contiguous block of 16kb of Spectrum RAM. Only
         * the first 6912 bytes is the display file.
         */
        [[nodiscard]] std::uint8_t * displayMemory() const override
        {
            return memory()->pointerTo(DisplayMemoryOffset);
        }

        /**
         * The size, in bytes, of the display memory.
         *
         * @return 6912.
         */
        [[nodiscard]] int displayMemorySize() const override
        {
            return DisplayMemorySize;
        }

    protected:
        /**
         * Load the given file as a ROM image into the low 16kb of the address space.
         *
         * @param romFile
         * @return
         */
        bool loadRom(const std::string & romFile);

        /**
         * Reloads the ROM image from disk.
         *
         * This is primarily used when the Spectrum is reset.
         */
        void reloadRoms() override;

    private:
        /**
         * The file on disk containing the Spectrum ROM image.
         */
        std::string m_romFile;
    };
}

#endif // SPECTRUM_SPECTRUM48K_H
