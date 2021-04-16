//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_SPECTRUMPLUS2A_H
#define SPECTRUM_SPECTRUMPLUS2A_H

#include "basespectrum.h"
#include "spectrumplus2amemory.h"
#include "spectrumplus2apagingdevice.h"

namespace Spectrum
{
    /**
     * Abstraction of a Spectrum +2a
     *
     * The Spectrum +2a's fundamentals are nearly identical to those of the Spectrum 48K: a Z80 running at roughly
     * 3.5MHz, 16kb of ROM in the lower 16kb of the Z80's address space and 48kb of read/write RAM in the remainder. The
     * main differences are:
     * - one of two available 16kb ROM images can be paged into the ROM area, at the programmer's discretion
     * - one of eight 16kb banks of RAM can be paged into the upper 16kb of the Z80 address space, at the programmer's
     *   discretion
     * - display double-buffering using a shadow display file in one of the alternative banks of memory
     * - a dedicated sound chip
     *
     * This class uses a custom Memory class to model the Spectrum 128K's memory state, and a Z80 IO device class to
     * represent the updated ULA functionality that handles paging in and out ROM and RAM and switching the display
     * file. See Spectrum128kMemory and Spectrum128KPagingDevice respectively.
     */
    class SpectrumPlus2a
    : public BaseSpectrum
    {
    public:
        using MemoryType = SpectrumPlus2aMemory;
        using ScreenBuffer = ScreenBuffer128k;

        static constexpr const int DisplayMemorySize = 6912;

        /**
         * Default constructor.
         *
         * Without any ROM images, the Spectrum will be fairly useless.
         */
        SpectrumPlus2a();

        /**
         * Initialise a new Spectrum +2a with ROM images loaded from disk.
         */
        explicit SpectrumPlus2a(const std::string & romFile0, const std::string & romFile1, const std::string & romFile2, const std::string & romFile3);

        // Spectrum128k instances can't be copy or move constructed or assigned
        SpectrumPlus2a(const SpectrumPlus2a &) = delete;
        SpectrumPlus2a(SpectrumPlus2a &&) = delete;
        void operator=(const SpectrumPlus2a &) = delete;
        void operator=(SpectrumPlus2a &&) = delete;

        /**
         * Destructor.
         */
        ~SpectrumPlus2a() override;

        /**
         * The Spectrum model type.
         *
         * Always Model::SpectrumPlus2a.
         *
         * @return
         */
        [[nodiscard]] inline constexpr Model model() const override
        {
            return Model::SpectrumPlus2a;
        }

        /**
         * Take a snapshot of the +2a state.
         *
         * @return A snapshot, or nullptr if a snapshot can't be created.
         */
        [[nodiscard]] std::unique_ptr<Snapshot> snapshot() const override;

        /**
         * Check whether a snapshot can be applied to a Spectrum +2a.
         *
         * The snapshot can be applied if its model is SpectrumPlus2a, it has an appropriate memory object and it has
         * an appropriate paged ROM number.
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
         * Fetch the display memory.
         *
         * The returned pointer is guaranteed to be a contiguous array of at least 6912 bytes representing the
         * Spectrum's display file. It takes account of the currently switched page file (normal or shadow) for double-
         * buffering.
         *
         * @return
         */
        [[nodiscard]] ::Z80::UnsignedByte * displayMemory() const override;

        /**
         * The size of the display file in bytes.
         *
         * @return 6912
         */
        [[nodiscard]] int displayMemorySize() const override
        {
            return DisplayMemorySize;
        }

        /**
         * Determine which screen buffer is currently active.
         *
         * @return
         */
        [[nodiscard]] inline ScreenBuffer screenBuffer() const
        {
            return m_screenBuffer;
        }

        /**
         * Set which screen buffer is currently active.
         *
         * You should never need to call this directly, unless you are reimplementing the SpectrumPlus2aPagingDevice
         * functionality.
         *
         * @param buffer
         */
        inline void setScreenBuffer(ScreenBuffer buffer)
        {
            m_screenBuffer = buffer;
        }

        /**
         * Fetch a read-only pointer to the device controlling memory paging for the Spectrum.
         *
         * The pointer is only valid as long as the Spectrum is alive. The Spectrum owns the pointer and it must not
         * be destroyed.
         *
         * @return A pointer to the SpectrumPlus2aPagingDevice.
         */
        [[nodiscard]] inline const SpectrumPlus2aPagingDevice * pager() const
        {
            return &m_pager;
        }

        /**
         * Fetch a read-write pointer to the device controlling memory paging for the Spectrum.
         *
         * The pointer is only valid as long as the Spectrum is alive. The Spectrum owns the pointer and it must not
         * be destroyed.
         *
         * @return A pointer to the SpectrumPlus2aPagingDevice.
         */
        [[nodiscard]] inline SpectrumPlus2aPagingDevice * pager()
        {
            return &m_pager;
        }

        /**
         * Reset the Spectrum.
         *
         * In addition to the standard Spectrum reset (memory cleared, registers reset, etc.), the screen buffer will be
         * set to the normal buffer, the paging device will be reset and the memory will be reset to its initial state
         * (ROM0 and Bank0 paged in).
         */
        void reset() override;

    protected:
        /**
         * Reload the ROM images from disk.
         *
         * Both ROMs will be reloaded.
         */
        void reloadRoms() override;

        /**
         * Helper to fetch the memory cast to the appropriate subclass to save re-writing the cast everwhere the memory
         * is used.
         *
         * @return
         */
        [[nodiscard]] inline SpectrumPlus2aMemory * memoryPlus2a() const
        {
            return dynamic_cast<SpectrumPlus2aMemory *>(memory());
        }

    private:
        /**
         * The device instance to handle memory paging.
         */
        SpectrumPlus2aPagingDevice m_pager;

        /**
         * The current screen buffer.
         */
        ScreenBuffer m_screenBuffer;

        /**
         * The file names of the two ROM images.
         */
        std::string m_romFiles[4];
    };
}

#endif //SPECTRUM_SPECTRUMPLUS2A_H
