//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_SNAPSHOT_H
#define SPECTRUM_SNAPSHOT_H

#include <variant>
#include "../simplememory.h"
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
     * when loading a snapshot file to apply to a Spectrum instance. It contains the model type from which the snapshot
     * was originally captured, a memory image, a full set of Z80 registers, the Z80 interrupt state and the Spectrum
     * border colour.
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
        MemoryBankNumber128k pagedBankNumber;

        /**
         * For 128k models, the paged-in ROM.
         */
        std::variant<RomNumber128k, RomNumberPlus2a> romNumber;

        /**
         * Create a snapshot of the current state of a Spectrum.
         *
         * The snapshot will take a copy of the Spectrum's memory, registers and CPU state. If the spectrum has at least
         * one display device, the border colour from the first display device will be stored.
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

        /**
         * Check whether the snapshot can be applied to the provided Spectrum.
         *
         * The two are compatible if all of the following are true:
         * - the snapshot's model is the same as the Spectrum's model
         * - the snapshot contains a memory object
         * - the snapshot's stored memory type is the same as that used by the specific Spectrum subclass
         *
         * If the two are not compatible, you will need to create a new Spectrum object based on the model() of the
         * snapshot.
         *
         * @return true if the Spectrum and Snapshot are compatible, false otherwise.
         */
        [[nodiscard]] bool canApplyTo(BaseSpectrum &) const;

        /**
         * Attempt to apply the snapshot to the provided Spectrum.
         *
         * It is the caller's responsibility to ensure that the provided Spectrum is suitable for the snapshot. See
         * canApplyTo().
         */
        void applyTo(BaseSpectrum &) const;

        /**
         * Populate the snapshot from the provided Spectrum.
         *
         * The spectrum's memory, registers and CPU state will be copied into the snapshot.
         */
        void readFrom(BaseSpectrum &);

        /**
         * Save the snapshot to a file using a specified writer class.
         *
         * On success the file specified will be overwritten.
         *
         * @tparam Writer The writer class to use.
         * @param fileName The path to the file to save to.
         * @return
         */
        template <class Writer>
        [[nodiscard]] bool saveAs(const std::string & fileName) const
        {
            static_assert(std::is_base_of_v<Io::SnapshotWriter, Writer>, "writer class must be a SnapshotWriter subclass");
            return Writer(*this).writeTo(fileName);
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
}

#endif //SPECTRUM_SNAPSHOT_H
