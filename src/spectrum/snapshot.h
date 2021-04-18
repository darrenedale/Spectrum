//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_SNAPSHOT_H
#define SPECTRUM_SNAPSHOT_H

#include <variant>
#include "../z80/registers.h"
#include "../z80/z80.h"
#include "types.h"
#include "basespectrum.h"

namespace Spectrum
{
    namespace Io
    {
        class SnapshotWriter;
    }

    /**
     * A representation of the state of a Spectrum at a point in time.
     *
     * The snapshot class is used to capture the state to store in a snapshot file when saving and to recreate the state
     * when loading a snapshot file to apply to a Spectrum instance. It contains:
     * - the model type from which the snapshot was originally captured
     * - a memory object
     * - a full set of Z80 registers
     * - the Z80 interrupt state
     * - the Spectrum border colour if it had any display devices attached when created
     * - the paged ROM and RAM bank (for 128k models)
     * - whether or not paging has been disabled (for 128k models)
     * - which screen buffer is in use (for 128k models)
     * - paging mode and special paging config (for +2a/+3 models)
     */
    class Snapshot
    {
    public:
        using Registers = ::Z80::Registers;

        /**
         * The colour of the Spectrum's border.
         *
         * This member is read-write for convenience and speed of access. It has no invariants or preconditions.
         */
        Colour border;

        /**
         * The primary interrupt flip-flops.
         *
         * This member is read-write for convenience and speed of access. It has no invariants or preconditions.
         */
        bool iff1;

        /**
         * The shadow interrupt flip-flops.
         *
         * This member is read-write for convenience and speed of access. It has no invariants or preconditions.
         */
        bool iff2;

        /**
         * The interrupt mode.
         *
         * This member is read-write for convenience and speed of access. It has no invariants or preconditions.
         */
        ::Z80::InterruptMode im;

        /**
         * For 128k models, the paged-in memory bank.
         */
        std::uint8_t pagedBankNumber;

        /**
         * For 128k models, the paged-in ROM.
         */
        std::uint8_t romNumber;

        /**
         * For 128K models, which screen buffer is currently in use.
         */
        ScreenBuffer128k screenBuffer;

        /**
         * For 128K models, whether or not paging is enabled.
         */
        bool pagingEnabled;

        /**
         * For +2a/+3, which paging mode is in use.
         */
        PagingMode pagingMode;

        /**
         * For +2a/+3 special paging mode, which paging configuration is in use.
         */
        SpecialPagingConfiguration specialPagingConfig;

        /**
         * Create a snapshot of the current state of a Spectrum.
         *
         * The snapshot will take a copy of the Spectrum's memory, registers and CPU state. If the spectrum has at least
         * one display device, the border colour from the first display device will be stored.
         *
         * Note that only the "lowest common denominator" properties of the snapshot are set - those properties that are
         * common to all Spectrums, as defined in the BaseSpectrum class. Individual models will need to set additional
         * state in the Snapshot (such as memory paging details) as appropriate.
         */
        explicit Snapshot(const BaseSpectrum &);

        /**
         * Create a snapshot for a specific model.
         *
         * An empty snapshot for the given model will be created. A Default-constructed set of registers will be stored,
         * the IFFs will be false, the interrupt mode will be 0 and the border colour will be white. No memory object
         * will be stored in the snapshot.
         *
         * The model defaults to Spectrum 48K to enable objects to be default-constructed but you are strongly
         * recommended to always provide a model explicitly.
         */
        explicit Snapshot(Model = Model::Spectrum48k);

        /**
         * Copy constructor.
         */
        Snapshot(const Snapshot &);

        /**
         * Move constructor.
         */
        Snapshot(Snapshot &&) noexcept;

        /**
         * Copy assignment operator.
         *
         * @return A reference to this.
         */
        Snapshot & operator=(const Snapshot &);

        /**
         * Move assignment operator.
         *
         * @return A reference to this.
         */
        Snapshot & operator=(Snapshot &&) noexcept;

        /**
         * Destructor.
         */
        virtual ~Snapshot() noexcept;

        /**
         * Fetch the model represented by the snapshot.
         *
         * The model is read-only to help prevent invalid snapshots from being created. In all uses cases, the model is
         * known when the snapshot is created - either when reading a snapshot file or creating a new snapshot from an
         * existing Spectrum - so there should be no need to set the model after construction.
         *
         * @return
         */
        [[nodiscard]] Model model() const
        {
            return m_model;
        }

       /**
        * Fetch a const reference to the registers stored in the snapshot.
        *
        * The registers returned are read-only.
        *
        * @return
        */
        [[nodiscard]] const Registers & registers() const
        {
            return m_registers;
        }

        /**
         * Fetch a reference to the registers stored in the snapshot.
         *
         * The register values can be altered using the returned reference.
         *
         * @return
         */
        Registers & registers()
        {
            return m_registers;
        }

        /**
         * Fetch a pointer to the memory object stored in the snapshot.
         *
         * The returned pointer belongs to the snapshot and must not be destroyed. The snapshot might not contain a
         * memory object, in which case the return value will be nullptr.
         *
         * @return
         */
        [[nodiscard]] const BaseSpectrum::MemoryType * memory() const
        {
            return m_memory.get();
        }

        /**
         * @param memory
         */
        void setMemory(std::unique_ptr<BaseSpectrum::MemoryType> && memory)
        {
            m_memory = std::move(memory);
        }

    private:
        /**
         * Internal constructor that the public constructors delegate to.
         *
         * @param registers the state of the Z80 registers to store in the snapshot.
         * @param memory A pointer to the memory object to store in the snapshot. The memory will be cloned.
         */
        explicit Snapshot(::Z80::Registers registers, BaseSpectrum::MemoryType * memory = nullptr);

        /**
         * The original Spectrum model the snapshot was created from.
         */
        Model m_model;

        /**
         * The Z80 registers.
         */
        Registers m_registers;

        /**
         * The memory of the Spectrum from when the snapshot was taken.
         */
        std::unique_ptr<BaseSpectrum::MemoryType> m_memory;
    };

#if (!defined(NDEBUG))
    std::ostream & operator<<(std::ostream & out, const Snapshot & snap);
#endif
}

#endif //SPECTRUM_SNAPSHOT_H
