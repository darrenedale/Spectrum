//
// Created by darren on 27/02/2021.
//

#ifndef Z80_REGISTERS_H
#define Z80_REGISTERS_H

#include "types.h"

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
            if constexpr (Z80ByteOrder == HostByteOrder) {
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
            if constexpr (Z80ByteOrder == HostByteOrder) {
                return *(reinterpret_cast<SignedWord *>(&native));
            }

            auto temp = swapByteOrder(native);
            return *(reinterpret_cast<SignedWord *>(&temp));
        }

        bool operator==(UnsignedWord value) const
        {
            if constexpr (Z80ByteOrder == HostByteOrder) {
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
            if constexpr (Z80ByteOrder == HostByteOrder) {
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
            if constexpr (Z80ByteOrder == HostByteOrder) {
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
            if constexpr (Z80ByteOrder == HostByteOrder) {
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
            if constexpr (Z80ByteOrder == HostByteOrder) {
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

        void reset()
        {
            bc = de = hl = pc = bcShadow = deShadow = hlShadow = ix = iy = 0x0000;
            af = afShadow = sp = 0xffff;
        }
        
        /**
         * Fetch a read-write reference to the AF register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian afZ80()
        {
            return {af};
        }

        /**
         * Fetch a read-write reference to the BC register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian bcZ80()
        {
            return {bc};
        }

        /**
         * Fetch a read-write reference to the DE register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian deZ80()
        {
            return {de};
        }

        /**
         * Fetch a read-write reference to the HL register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian hlZ80()
        {
            return {hl};
        }

        /**
         * Fetch a read-write reference to the IX register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian ixZ80()
        {
            return {ix};
        }

        /**
         * Fetch a read-write reference to the IY register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian iyZ80()
        {
            return {iy};
        }

        /**
         * Fetch a read-write reference to the PC (program counter) register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian pcZ80()
        {
            return {pc};
        }

        /**
         * Fetch a read-write reference to the SP register (stack pointer) in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian spZ80()
        {
            return {sp};
        }

        /**
         * Fetch a read-write reference to the AF shadow register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian afZ80Shadow()
        {
            return {afShadow};
        }

        /**
         * Fetch a read-write reference to the BC shadow register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian bcZ80Shadow()
        {
            return {bcShadow};
        }

        /**
         * Fetch a read-write reference to the DE shadow register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian deZ80Shadow()
        {
            return {deShadow};
        }

        /**
         * Fetch a read-write reference to the HL shadow register in Z80 byte order.
         *
         * @return 
         */
        RegisterZ80Endian hlZ80Shadow()
        {
            return {hlShadow};
        }

        /**
         * Fetch a read-write reference to the A register.
         *
         * @return
         */
        UnsignedByte & a()
        {
            return lowRegister(af);
        }

        /**
         * Fetch the value of the A register.
         *
         * @return
         */
        UnsignedByte a() const
        {
            return lowRegister(af);
        }

        /**
         * Fetch a read-write reference to the F register.
         *
         * @return
         */
        UnsignedByte & f()
        {
            return highRegister(af);
        }

        /**
         * Fetch the value of the F register.
         *
         * @return
         */
        UnsignedByte f() const
        {
            return highRegister(af);
        }

        /**
         * Fetch a read-write reference to the B register.
         *
         * @return
         */
        UnsignedByte & b()
        {
            return lowRegister(bc);
        }

        /**
         * Fetch the value of the B register.
         *
         * @return
         */
        UnsignedByte b() const
        {
            return lowRegister(bc);
        }

        /**
         * Fetch a read-write reference to the C register.
         *
         * @return
         */
        UnsignedByte & c()
        {
            return highRegister(bc);
        }

        /**
         * Fetch the value of the C register.
         *
         * @return
         */
        UnsignedByte c() const
        {
            return highRegister(bc);
        }

        /**
         * Fetch a read-write reference to the D register.
         *
         * @return
         */
        UnsignedByte & d()
        {
            return lowRegister(de);
        }

        /**
         * Fetch the value of the D register.
         *
         * @return
         */
        UnsignedByte d() const
        {
            return lowRegister(de);
        }

        /**
         * Fetch a read-write reference to the E register.
         *
         * @return
         */
        UnsignedByte & e()
        {
            return highRegister(de);
        }

        /**
         * Fetch the value of the E register.
         *
         * @return
         */
        UnsignedByte e() const
        {
            return highRegister(de);
        }

        /**
         * Fetch a read-write reference to the H register.
         *
         * @return
         */
        UnsignedByte & h()
        {
            return lowRegister(hl);
        }

        /**
         * Fetch the value of the H register.
         *
         * @return
         */
        UnsignedByte h() const
        {
            return lowRegister(hl);
        }

        /**
         * Fetch a read-write reference to the L register.
         *
         * @return
         */
        UnsignedByte & l()
        {
            return highRegister(hl);
        }

        /**
         * Fetch the value of the L register.
         *
         * @return
         */
        UnsignedByte l() const
        {
            return highRegister(hl);
        }

        /**
         * Fetch a read-write reference to the L register.
         *
         * @return
         */
        UnsignedByte & ixl()
        {
            return lowRegister(ix);
        }

        /**
         * Fetch the value of the L register.
         *
         * @return
         */
        UnsignedByte ixl() const
        {
            return lowRegister(ix);
        }

        /**
         * Fetch a read-write reference to the high byte of the IX register.
         *
         * @return
         */
        UnsignedByte & ixh()
        {
            return highRegister(ix);
        }

        /**
         * Fetch the value of the high byte of the IX register.
         *
         * @return
         */
        UnsignedByte ixh() const
        {
            return highRegister(ix);
        }

        /**
         * Fetch a read-write reference to the low byte of the IY register.
         *
         * @return
         */
        UnsignedByte & iyl()
        {
            return lowRegister(iy);
        }

        /**
         * Fetch the value of the low byte of the IY register.
         *
         * @return
         */
        UnsignedByte iyl() const
        {
            return lowRegister(iy);
        }

        /**
         * Fetch a read-write reference to the high byte of the IY register.
         *
         * @return
         */
        UnsignedByte & iyh()
        {
            return highRegister(iy);
        }

        /**
         * Fetch the value of the high byte of the IY register.
         *
         * @return
         */
        UnsignedByte iyh() const
        {
            return highRegister(iy);
        }

        /**
         * Fetch a read-write reference to the A shadow register.
         *
         * @return
         */
        UnsignedByte & aShadow()
        {
            return lowRegister(afShadow);
        }

        /**
         * Fetch the value of the A shadow register.
         *
         * @return
         */
        UnsignedByte aShadow() const
        {
            return lowRegister(afShadow);
        }

        /**
         * Fetch a read-write reference to the F shadow register.
         *
         * @return
         */
        UnsignedByte & fShadow()
        {
            return highRegister(afShadow);
        }

        /**
         * Fetch the value of the F shadow register.
         *
         * @return
         */
        UnsignedByte fShadow() const
        {
            return highRegister(afShadow);
        }

        /**
         * Fetch a read-write reference to the B shadow register.
         *
         * @return
         */
        UnsignedByte & bShadow()
        {
            return lowRegister(bcShadow);
        }

        /**
         * Fetch the value of the B shadow register.
         *
         * @return
         */
        UnsignedByte bShadow() const
        {
            return lowRegister(bcShadow);
        }

        /**
         * Fetch a read-write reference to the C shadow register.
         *
         * @return
         */
        UnsignedByte & cShadow()
        {
            return highRegister(bcShadow);
        }

        /**
         * Fetch the value of the C shadow register.
         *
         * @return
         */
        UnsignedByte cShadow() const
        {
            return highRegister(bcShadow);
        }

        /**
         * Fetch a read-write reference to the D shadow register.
         *
         * @return
         */
        UnsignedByte & dShadow()
        {
            return lowRegister(deShadow);
        }

        /**
         * Fetch the value of the D shadow register.
         *
         * @return
         */
        UnsignedByte dShadow() const
        {
            return lowRegister(deShadow);
        }

        /**
         * Fetch a read-write reference to the E shadow register.
         *
         * @return
         */
        UnsignedByte & eShadow()
        {
            return highRegister(deShadow);
        }

        /**
         * Fetch the value of the E shadow register.
         *
         * @return
         */
        UnsignedByte eShadow() const
        {
            return highRegister(deShadow);
        }

        /**
         * Fetch a read-write reference to the H shadow register.
         *
         * @return
         */
        UnsignedByte & hShadow()
        {
            return lowRegister(hlShadow);
        }

        /**
         * Fetch the value of the H shadow register.
         *
         * @return
         */
        UnsignedByte hShadow() const
        {
            return lowRegister(hlShadow);
        }

        /**
         * Fetch a read-write reference to the L shadow register.
         *
         * @return
         */
        UnsignedByte & lShadow()
        {
            return highRegister(hlShadow);
        }

        /**
         * Fetch the value of the L shadow register.
         *
         * @return
         */
        UnsignedByte lShadow() const
        {
            return highRegister(hlShadow);
        }

    private:
        /**
         * Helper to fetch a reference to the low byte of a 16-bit register.
         *
         * @param registerPair
         * @return
         */
        static inline UnsignedByte & lowRegister(UnsignedWord & registerPair)
        {
            if constexpr(Z80ByteOrder == HostByteOrder) {
                return (reinterpret_cast<UnsignedByte *>(&registerPair))[1];
            }

            return (reinterpret_cast<UnsignedByte *>(&registerPair))[0];
        }

        /**
         * Helper to fetch a reference to the low byte of a 16-bit register.
         *
         * @param registerPair
         * @return
         */
        static inline UnsignedByte lowRegister(const UnsignedWord & registerPair)
        {
            if constexpr(Z80ByteOrder == HostByteOrder) {
                return (reinterpret_cast<const UnsignedByte *>(&registerPair))[1];
            }

            return (reinterpret_cast<const UnsignedByte *>(&registerPair))[0];
        }

        /**
         * Helper to fetch a reference to the high byte of a 16-bit register.
         *
         * @param registerPair
         * @return
         */
        static inline UnsignedByte & highRegister(UnsignedWord & registerPair)
        {
            if constexpr(Z80ByteOrder == HostByteOrder) {
                return (reinterpret_cast<UnsignedByte *>(&registerPair))[0];
            }

            return (reinterpret_cast<UnsignedByte *>(&registerPair))[1];
        }

        /**
         * Helper to fetch a reference to the high byte of a 16-bit register.
         *
         * @param registerPair
         * @return
         */
        static inline UnsignedByte highRegister(const UnsignedWord & registerPair)
        {
            if constexpr(Z80ByteOrder == HostByteOrder) {
                return (reinterpret_cast<const UnsignedByte *>(&registerPair))[0];
            }

            return (reinterpret_cast<const UnsignedByte *>(&registerPair))[1];
        }
    };

}

#endif //Z80_REGISTERS_H
