//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_SNAPSHOT_H
#define SPECTRUM_SNAPSHOT_H

#include "../z80/registers.h"
#include "../z80/z80.h"
#include "types.h"
#include "basespectrum.h"

namespace Spectrum
{
    class Snapshot
    {
    public:
        struct Memory
        {
            Z80::UnsignedByte * image = nullptr;
            int size = 0xffff;
        };

        /**
         * Plain, trivially copyable and assignable set of registers.
         *
         * NOTE we can't use the Z80::Registers struct because it can't be assigned or copied only moved because of the
         * internal references it keeps.
         */
        struct Registers
        {
            ::Z80::UnsignedWord af;
            ::Z80::UnsignedWord bc;
            ::Z80::UnsignedWord de;
            ::Z80::UnsignedWord hl;
            ::Z80::UnsignedWord ix;
            ::Z80::UnsignedWord iy;
            ::Z80::UnsignedWord pc;
            ::Z80::UnsignedWord sp;

            ::Z80::UnsignedWord afShadow;
            ::Z80::UnsignedWord bcShadow;
            ::Z80::UnsignedWord deShadow;
            ::Z80::UnsignedWord hlShadow;

            ::Z80::UnsignedWord memptr;

            ::Z80::UnsignedByte i;
            ::Z80::UnsignedByte r;

            ::Z80::UnsignedByte a;
            ::Z80::UnsignedByte f;
            ::Z80::UnsignedByte b;
            ::Z80::UnsignedByte c;
            ::Z80::UnsignedByte d;
            ::Z80::UnsignedByte e;
            ::Z80::UnsignedByte h;
            ::Z80::UnsignedByte l;

            ::Z80::UnsignedByte aShadow;
            ::Z80::UnsignedByte fShadow;
            ::Z80::UnsignedByte bShadow;
            ::Z80::UnsignedByte cShadow;
            ::Z80::UnsignedByte dShadow;
            ::Z80::UnsignedByte eShadow;
            ::Z80::UnsignedByte hShadow;
            ::Z80::UnsignedByte lShadow;
        };

        Colour border;
        bool iff1;
        bool iff2;
        ::Z80::InterruptMode im;

        /**
         * The snapshot will take a copy of the Spectrum's memory and its CPU state.
         */
        explicit Snapshot(const BaseSpectrum &);

        /**
         * The snapshot will take a copy of the provided memory and will default-initialise a CPU state.
         */
        explicit Snapshot(Z80::UnsignedByte * memory = nullptr, int memorySize = 0xffff);
        Snapshot(const Snapshot &);
        Snapshot(Snapshot &&) noexcept;
        Snapshot & operator=(const Snapshot &);
        Snapshot & operator=(Snapshot &&) noexcept;
        virtual ~Snapshot() noexcept;

        [[nodiscard]] const Registers & registers() const
        {
            return m_registers;
        }

        Registers & registers()
        {
            return m_registers;
        }

        [[nodiscard]] const Memory & memory() const
        {
            return m_memory;
        }

        void applyTo(BaseSpectrum &) const;
        void readFrom(BaseSpectrum &);

        template <class Writer>
        bool saveAs(const std::string & fileName) const
        {
            return Writer(*this).writeTo(fileName);
        }

    private:
        explicit Snapshot(const ::Z80::Registers &, Z80::UnsignedByte * memory = nullptr, int memorySize = 0xffff);
        void copyRegisters(const ::Z80::Registers &);
        struct Registers m_registers;
        Memory m_memory;
    };
}

#endif //SPECTRUM_SNAPSHOT_H
