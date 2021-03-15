//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_SNAPSHOT_H
#define SPECTRUM_SNAPSHOT_H

#include "../z80/registers.h"
#include "../z80/z80.h"
#include "spectrum.h"

namespace Spectrum
{
    class Snapshot
    {
    public:
        using Registers = ::Z80::Registers;

        struct Memory
        {
            Z80::UnsignedByte * image = nullptr;
            int size = 0xffff;
        };

        bool iff1;
        bool iff2;
        Z80::Z80::InterruptMode im;

        explicit Snapshot(Memory memory);
        Snapshot(Registers registers, Memory memory);
        explicit Snapshot(Z80::UnsignedByte * memory = nullptr, int memorySize = 0xffff);
        explicit Snapshot(Registers , Z80::UnsignedByte * memory = nullptr, int memorySize = 0xffff);
        Snapshot(const Snapshot &);
        Snapshot(Snapshot &&) noexcept;
//        Snapshot & operator=(const Snapshot &);
//        Snapshot & operator=(Snapshot &&);
        virtual ~Snapshot() noexcept;

        [[nodiscard]] const Registers & registers() const
        {
            return m_registers;
        }

        Registers & registers()
        {
            return m_registers;
        }

        void applyTo(Spectrum &) const;
        void readFrom(Spectrum &);

    private:
        Registers m_registers;
        Memory m_memory;
    };
}

#endif //SPECTRUM_SNAPSHOT_H
