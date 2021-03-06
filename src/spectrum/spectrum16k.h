//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_SPECTRUM16K_H
#define SPECTRUM_SPECTRUM16K_H

#include <string>
#include "basespectrum.h"
#include "memory.h"

namespace Spectrum
{
    /**
     * Abstraction of a Spectrum 16K
     *
     * A 16K spectrum is almost identical to a 48K spectrum except only half the Z80's address space is occupied with
     * actual memory. The remainder of the address space can be read and written but it has no effect.
     *
     * The 6912 bytes of memory at 0x4000 (16384) - right at the beginning of the RAM segment of the address space - is
     * the display file, basically the Spectrum's VRAM.
     */
    class Spectrum16k
    : public BaseSpectrum
    {
    public:
        using MemoryType = Spectrum::Memory;
        static constexpr const int DisplayMemoryOffset = 0x4000;

        /**
         * Default constructor.
         *
         * Without a loaded ROM, the Spectrum is not likely to be much use.
         */
        Spectrum16k();

        /**
         * Initialise a new Spectrum48k with a ROM image on disk.
         *
         * Note that in the real world the ROM for a Spectrum 16K is the same as the ROM for a Spectrum 48K.
         *
         * @param romFile
         */
        explicit Spectrum16k(const std::string & romFile);

        // Spectrum16k's are not copy or move constructable or assignable
        Spectrum16k(const Spectrum16k &) = delete;
        Spectrum16k(Spectrum16k &&) = delete;
        void operator=(const Spectrum16k &) = delete;
        void operator=(Spectrum16k &&) = delete;

        /**
         * Destructor.
         */
        ~Spectrum16k() override;

        /**
         * The Spectrum model type.
         *
         * Always Model::Spectrum16k.
         *
         * @return
         */
        [[nodiscard]] inline constexpr Model model() const override
        {
            return Model::Spectrum16k;
        }

        /**
         * Take a snapshot of the Spectrum 16K.
         *
         * @return A Snapshot, or nullptr if the snapshot can't be taken.
         */
        [[nodiscard]] std::unique_ptr<Snapshot> snapshot() const override;

        /**
         * Check whether a snapshot can be applied to a 16K Spectrum.
         *
         * The snapshot can be applied if its model is Spectrum16K and it has an appropriate memory object.
         *
         * @param snapshot The Snapshot to check.
         * @return true if the snapshot can be applied to this Spectrum, false otherwise.
         */
        [[nodiscard]] bool canApplySnapshot(const Snapshot &snapshot) override;

        /**
         * Apply the provided snapshot to this Spectrum.
         *
         * The caller must ensure that the snapshot is compatible before calling this method.
         *
         * @param snapshot The snapshot to apply.
         */
        void applySnapshot(const Snapshot &snapshot) override;

        /**
         * Fetch the Spectrum's display file.
         *
         * @return The Spectrum's current display file memory.
         */
        [[nodiscard]] DisplayFile displayMemory() const override
        {
            return DisplayFile(memory()->pointerTo(DisplayMemoryOffset), DisplayFile::extent);
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

#endif //SPECTRUM_SPECTRUM16K_H
