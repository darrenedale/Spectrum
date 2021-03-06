//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_SPECTRUM128K_H
#define SPECTRUM_SPECTRUM128K_H

#include "basespectrum.h"
#include "memory128k.h"
#include "pagingdevice128k.h"

namespace Spectrum
{
    /**
     * Abstraction of a Spectrum 128K
     *
     * The Spectrum 128K's fundamentals are nearly identical to those of the Spectrum 48K: a Z80 running at roughly
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
    class Spectrum128k
    : public BaseSpectrum
    {
    public:
        using MemoryType = Memory128k;
        using ScreenBuffer = ScreenBuffer128k;

        /**
         * Default constructor.
         *
         * Without any ROM images, the Spectrum will be fairly useless.
         */
        Spectrum128k();

        /**
         * Initialise a new Spectrum128k with ROM images loaded from disk.
         */
        Spectrum128k(const std::string & romFile0, const std::string & romFile1);

        // Spectrum128k instances can't be copy or move constructed or assigned
        Spectrum128k(const Spectrum128k &) = delete;
        Spectrum128k(Spectrum128k &&) = delete;
        void operator=(const Spectrum128k &) = delete;
        void operator=(Spectrum128k &&) = delete;
        ~Spectrum128k() override;

        /**
         * The Spectrum model type.
         *
         * @return Always Model::Spectrum128k.
         */
        [[nodiscard]] inline constexpr Model model() const override
        {
            return Model::Spectrum128k;
        }

        /**
         * Take a snapshot of the Spectrum 128K state.
         *
         * @return A snapshot, or nullptr if the snapshot could not be taken.
         */
        [[nodiscard]] std::unique_ptr<Snapshot> snapshot() const override;

        /**
         * Check whether a snapshot can be applied to a 128K Spectrum.
         *
         * The snapshot can be applied if its model is Spectrum128K, it has an appropriate memory object and it has
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
         * Fetch the Spectrum's display file.
         *
         * The returned span takes account of the currently switched page file (normal or shadow) for double- buffering.
         *
         * @return The Spectrum's current display file memory.
         */
        [[nodiscard]] DisplayFile displayMemory() const override;

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
         * You should never need to call this directly, unless you are reimplementing the Spectrum128KPagingDevice
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
         * @return A pointer to the Spectrum128KPagingDevice.
         */
        [[nodiscard]] inline const PagingDevice128k * pager() const
        {
            return &m_pager;
        }

        /**
         * Fetch a read-write pointer to the device controlling memory paging for the Spectrum.
         *
         * The pointer is only valid as long as the Spectrum is alive. The Spectrum owns the pointer and it must not
         * be destroyed.
         *
         * @return A pointer to the Spectrum128KPagingDevice.
         */
        [[nodiscard]] inline PagingDevice128k * pager()
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
        [[nodiscard]] inline Memory128k * memory128() const
        {
            return dynamic_cast<Memory128k *>(memory());
        }

    private:
        /**
         * The device instance to handle memory paging.
         */
        PagingDevice128k m_pager;

        /**
         * The current screen buffer.
         */
        ScreenBuffer m_screenBuffer;

        /**
         * The file names of the two ROM images.
         */
        std::string m_romFiles[2];
    };
}

#endif //SPECTRUM_SPECTRUM128K_H
