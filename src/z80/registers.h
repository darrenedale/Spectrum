//
// Created by darren on 27/02/2021.
//

#ifndef Z80_REGISTERS_H
#define Z80_REGISTERS_H

#include "types.h"

namespace
{
    constexpr const bool ByteOrderMatch = (Z80::HostByteOrder == Z80::Z80ByteOrder);
};

namespace Z80
{
//#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
//    using RegisterZ80Endian = Z80::UnsignedWord &;
//#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    /**
     * Utility class to abstract away differences in byte order between the host and Z80 platforms. If the host and
     * Z80 use different byte orders, instances of this class enable reading and writing of registers using Z80 byte
     * order without having to perform any branches and without having to do use any byte-swapping functions.
     *
     * Objects support addition, subtraction and comparison. All arithmetic and comparisons assume Z80 byte order - so
     * if you add an UnsignedWord to a register, the word you provide is assumed to be in Z80 order, and it will be
     * appropriately converted before it is added to the original register (which is stored in host byte order).
     *
     * Note that objects of this class are only valid for the lifetime of the Z80 that created them. Once the Z80 goes
     * out of scope, the register object is no longer valid.
     */
    struct RegisterZ80Endian
    {
        UnsignedWord & native;

        /**
         * Obtain the register value as a Z80 byte-order 16-bit unsigned scalar.
         *
         * @return
         */
        operator UnsignedWord() const
        {
            if constexpr (ByteOrderMatch) {
                return native;
            }

            return swapByteOrder(native);
        }

        /**
         * Obtain the register value as a Z80 byte-order 16-bit signed scalar.
         *
         * @return
         */
        operator SignedWord() const
        {
            if constexpr (ByteOrderMatch) {
                return *(reinterpret_cast<SignedWord *>(&native));
            }

            auto temp = swapByteOrder(native);
            return *(reinterpret_cast<SignedWord *>(&temp));
        }

        bool operator==(UnsignedWord value) const
        {
            if constexpr (ByteOrderMatch) {
                return native == value;
            }

            return native == swapByteOrder(value);
        }

        bool operator==(RegisterZ80Endian other) const
        {
            return native == other.native;
        }

        int operator<=>(UnsignedWord value) const
        {
            if constexpr (ByteOrderMatch) {
                return native - value;
            }

            return native - swapByteOrder(value);
        }

        int operator<=>(RegisterZ80Endian other) const
        {
            return native - other.native;
        }

        RegisterZ80Endian & operator=(UnsignedWord value)
        {
            if constexpr (ByteOrderMatch) {
                native = value;
            } else {
                native = swapByteOrder(value);
            }

            return *this;
        }

        RegisterZ80Endian & operator=(RegisterZ80Endian other)
        {
            native = other.native;
            return *this;
        }

        RegisterZ80Endian & operator+(UnsignedWord value)
        {
            if constexpr (ByteOrderMatch) {
                native += value;
            } else {
                native += swapByteOrder(value);
            }

            return *this;
        }

        RegisterZ80Endian & operator+(RegisterZ80Endian other)
        {
            native += other.native;
            return *this;
        }

        RegisterZ80Endian & operator-(UnsignedWord value)
        {
            if constexpr (ByteOrderMatch) {
                native -= value;
            } else {
                native -= swapByteOrder(value);
            }

            return *this;
        }

        RegisterZ80Endian & operator-(RegisterZ80Endian other)
        {
            native -= other.native;
            return *this;
        }

        RegisterZ80Endian & operator++()
        {
            ++native;
            return *this;
        }

        RegisterZ80Endian & operator--()
        {
            --native;
            return *this;
        }

    private:
        /**
         * Helper to swap the byte order of values when host and Z80 byte orders differ.
         *
         * @param value
         * @return
         */
        constexpr static UnsignedWord swapByteOrder(UnsignedWord value)
        {
            return (value & 0xff00 >> 8) | (value & 0x00ff << 8);
        }
    };
//#endif

    /**
     * Set of Z80 registers.
     *
     * All values are stored in host byte order. Accessors are provided for all registers in Z80 byte order also, and
     * these are writable. Where host byte order and Z80 byte order differ, the Z80 accessors provide an instance of a
     * utility class that provides automatic conversion. Given that in some contexts we want to read/write registers in
     * Z80 byte order and in others we want to read/write in host byte order, this abstractions enables this to be done
     * transparently using a consistent interface regardless of the host platform.
     *
     * Host byte order rather than Z80 byte order is used for storage because it makes it easier, clearer, more
     * efficient and less error prone when doing things like incrementing the stack pointer, manipulating the program
     * counter, taking jumps etc.
     */
    struct Registers
    {
        UnsignedWord af;
        UnsignedWord bc;
        UnsignedWord de;
        UnsignedWord hl;
        UnsignedWord ix;
        UnsignedWord iy;
        UnsignedWord pc;
        UnsignedWord sp;

        UnsignedWord afShadow;
        UnsignedWord bcShadow;
        UnsignedWord deShadow;
        UnsignedWord hlShadow;

        UnsignedWord memptr;

        UnsignedByte i;
        UnsignedByte r;

        UnsignedByte & a = (*(reinterpret_cast<UnsignedByte *>(&af) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & f = (*(reinterpret_cast<UnsignedByte *>(&af) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & b = (*(reinterpret_cast<UnsignedByte *>(&bc) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & c = (*(reinterpret_cast<UnsignedByte *>(&bc) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & d = (*(reinterpret_cast<UnsignedByte *>(&de) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & e = (*(reinterpret_cast<UnsignedByte *>(&de) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & h = (*(reinterpret_cast<UnsignedByte *>(&hl) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & l = (*(reinterpret_cast<UnsignedByte *>(&hl) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & ixh = (*(reinterpret_cast<UnsignedByte *>(&ix) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & ixl = (*(reinterpret_cast<UnsignedByte *>(&ix) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & iyh = (*(reinterpret_cast<UnsignedByte *>(&iy) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & iyl = (*(reinterpret_cast<UnsignedByte *>(&iy) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & aShadow = (*(reinterpret_cast<UnsignedByte *>(&afShadow) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & fShadow = (*(reinterpret_cast<UnsignedByte *>(&afShadow) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & bShadow = (*(reinterpret_cast<UnsignedByte *>(&bcShadow) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & cShadow = (*(reinterpret_cast<UnsignedByte *>(&bcShadow) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & dShadow = (*(reinterpret_cast<UnsignedByte *>(&deShadow) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & eShadow = (*(reinterpret_cast<UnsignedByte *>(&deShadow) + (ByteOrderMatch ? 0 : 1)));
        UnsignedByte & hShadow = (*(reinterpret_cast<UnsignedByte *>(&hlShadow) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & lShadow = (*(reinterpret_cast<UnsignedByte *>(&hlShadow) + (ByteOrderMatch ? 0 : 1)));

        UnsignedByte & pcH = (*(reinterpret_cast<UnsignedByte *>(&pc) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & pcL = (*(reinterpret_cast<UnsignedByte *>(&pc) + (ByteOrderMatch ? 0 : 1)));

        UnsignedByte & spH = (*(reinterpret_cast<UnsignedByte *>(&sp) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & spL = (*(reinterpret_cast<UnsignedByte *>(&sp) + (ByteOrderMatch ? 0 : 1)));

        UnsignedByte & memptrH = (*(reinterpret_cast<UnsignedByte *>(&memptr) + (ByteOrderMatch ? 1 : 0)));
        UnsignedByte & memptrL = (*(reinterpret_cast<UnsignedByte *>(&memptr) + (ByteOrderMatch ? 0 : 1)));

        RegisterZ80Endian afZ80 = {af};
        RegisterZ80Endian bcZ80 = {bc};
        RegisterZ80Endian deZ80 = {de};
        RegisterZ80Endian hlZ80 = {hl};
        RegisterZ80Endian ixZ80 = {ix};
        RegisterZ80Endian iyZ80 = {iy};
        RegisterZ80Endian pcZ80 = {pc};
        RegisterZ80Endian spZ80 = {sp};

        RegisterZ80Endian afShadowZ80 = {afShadow};
        RegisterZ80Endian bcShadowZ80 = {bcShadow};
        RegisterZ80Endian deShadowZ80 = {deShadow};
        RegisterZ80Endian hlShadowZ80 = {hlShadow};

        RegisterZ80Endian memptrZ80 = {memptr};

        Registers() = default;
        Registers(Registers &&) = default;
        // copy constructor ensures references to high/low byte of register pairs are appropriately initialised to the
        // members in the new instance rather than retaining the reference to the members of the original instance
        Registers(const Registers & other)
        : af(other.af),
          bc(other.bc),
          de(other.de),
          hl(other.hl),
          ix(other.ix),
          iy(other.iy),
          pc(other.pc),
          sp(other.sp),
          afShadow(other.afShadow),
          bcShadow(other.bcShadow),
          deShadow(other.deShadow),
          hlShadow(other.hlShadow),
          memptr(other.memptr),
          i(other.i),
          r(other.r),
          a((*(reinterpret_cast<UnsignedByte *>(&af) + (ByteOrderMatch ? 1 : 0)))),
          f((*(reinterpret_cast<UnsignedByte *>(&af) + (ByteOrderMatch ? 0 : 1)))),
          b((*(reinterpret_cast<UnsignedByte *>(&bc) + (ByteOrderMatch ? 1 : 0)))),
          c((*(reinterpret_cast<UnsignedByte *>(&bc) + (ByteOrderMatch ? 0 : 1)))),
          d((*(reinterpret_cast<UnsignedByte *>(&de) + (ByteOrderMatch ? 1 : 0)))),
          e((*(reinterpret_cast<UnsignedByte *>(&de) + (ByteOrderMatch ? 0 : 1)))),
          h((*(reinterpret_cast<UnsignedByte *>(&hl) + (ByteOrderMatch ? 1 : 0)))),
          l((*(reinterpret_cast<UnsignedByte *>(&hl) + (ByteOrderMatch ? 0 : 1)))),
          ixh((*(reinterpret_cast<UnsignedByte *>(&ix) + (ByteOrderMatch ? 1 : 0)))),
          ixl((*(reinterpret_cast<UnsignedByte *>(&ix) + (ByteOrderMatch ? 0 : 1)))),
          iyh((*(reinterpret_cast<UnsignedByte *>(&iy) + (ByteOrderMatch ? 1 : 0)))),
          iyl((*(reinterpret_cast<UnsignedByte *>(&iy) + (ByteOrderMatch ? 0 : 1)))),
          aShadow((*(reinterpret_cast<UnsignedByte *>(&afShadow) + (ByteOrderMatch ? 1 : 0)))),
          fShadow((*(reinterpret_cast<UnsignedByte *>(&afShadow) + (ByteOrderMatch ? 0 : 1)))),
          bShadow((*(reinterpret_cast<UnsignedByte *>(&bcShadow) + (ByteOrderMatch ? 1 : 0)))),
          cShadow((*(reinterpret_cast<UnsignedByte *>(&bcShadow) + (ByteOrderMatch ? 0 : 1)))),
          dShadow((*(reinterpret_cast<UnsignedByte *>(&deShadow) + (ByteOrderMatch ? 1 : 0)))),
          eShadow((*(reinterpret_cast<UnsignedByte *>(&deShadow) + (ByteOrderMatch ? 0 : 1)))),
          hShadow((*(reinterpret_cast<UnsignedByte *>(&hlShadow) + (ByteOrderMatch ? 1 : 0)))),
          lShadow((*(reinterpret_cast<UnsignedByte *>(&hlShadow) + (ByteOrderMatch ? 0 : 1)))),
          pcH((*(reinterpret_cast<UnsignedByte *>(&pc) + (ByteOrderMatch ? 1 : 0)))),
          pcL((*(reinterpret_cast<UnsignedByte *>(&pc) + (ByteOrderMatch ? 0 : 1)))),
          spH((*(reinterpret_cast<UnsignedByte *>(&sp) + (ByteOrderMatch ? 1 : 0)))),
          spL((*(reinterpret_cast<UnsignedByte *>(&sp) + (ByteOrderMatch ? 0 : 1)))),
          memptrH((*(reinterpret_cast<UnsignedByte *>(&memptr) + (ByteOrderMatch ? 1 : 0)))),
          memptrL((*(reinterpret_cast<UnsignedByte *>(&memptr) + (ByteOrderMatch ? 0 : 1)))),
          afZ80{af},
          bcZ80{bc},
          deZ80{de},
          hlZ80{hl},
          ixZ80{ix},
          iyZ80{iy},
          pcZ80{pc},
          spZ80{sp},
          afShadowZ80{afShadow},
          bcShadowZ80{bcShadow},
          deShadowZ80{deShadow},
          hlShadowZ80{hlShadow},
          memptrZ80{memptr}
        {}

        void reset()
        {
            af = 0xffff;
            bc = 0x0000;
            de = 0x0000;
            hl = 0x0000;
            ix = 0x0000;
            iy = 0x0000;
            pc = 0x0000;
            sp = 0xffff;
            afShadow = 0xffff;
            bcShadow = 0x0000;
            deShadow = 0x0000;
            hlShadow = 0x0000;
            memptr = 0x0000;
            i = 0;
            r = 0;
        }
    };

}

#endif //Z80_REGISTERS_H
