#ifndef Z80_Z80_H
#define Z80_Z80_H

#include <cassert>
#include <cstdint>
#include <bit>
#include <set>
#include <iostream>

#include "../cpu.h"
#include "../simplememory.h"
#include "registers.h"
#include "types.h"
#include "endian.h"

#if (!defined(NDEBUG))
#include "executionhistory.h"
#endif

#define Z80_FLAG_C_BIT 0
#define Z80_FLAG_Z_BIT 6
#define Z80_FLAG_P_BIT 2
#define Z80_FLAG_S_BIT 7
#define Z80_FLAG_N_BIT 1
#define Z80_FLAG_H_BIT 4
#define Z80_FLAG_F3_BIT 3
#define Z80_FLAG_F5_BIT 5

#define Z80_FLAG_C_MASK (0x01 << Z80_FLAG_C_BIT)
#define Z80_FLAG_Z_MASK (0x01 << Z80_FLAG_Z_BIT)
#define Z80_FLAG_P_MASK (0x01 << Z80_FLAG_P_BIT)
#define Z80_FLAG_S_MASK (0x01 << Z80_FLAG_S_BIT)
#define Z80_FLAG_N_MASK (0x01 << Z80_FLAG_N_BIT)
#define Z80_FLAG_H_MASK (0x01 << Z80_FLAG_H_BIT)
#define Z80_FLAG_F3_MASK (0x01 << Z80_FLAG_F3_BIT)
#define Z80_FLAG_F5_MASK (0x01 << Z80_FLAG_F5_BIT)

namespace Z80
{
    class IODevice;

    class Z80
    : public Cpu
    {
    public:
        using Memory = ::Memory<UnsignedByte>;

        Z80(Memory * memory);
        ~Z80() override;

//        static inline UnsignedWord swapByteOrder(UnsignedWord value)
//        {
//            return (((value & 0xff00) >> 8) & 0x00ff) | (((value & 0x00ff) << 8) & 0xff00);
//        }
//
//        static inline UnsignedWord z80ToHostByteOrder(UnsignedWord value)
//        {
//            if constexpr (Z80ByteOrder == HostByteOrder) {
//                return value;
//            }
//
//            return swapByteOrder(value);
//        }
//
//        static inline UnsignedWord hostToZ80ByteOrder(UnsignedWord value)
//        {
//            if constexpr (Z80ByteOrder == HostByteOrder) {
//                return value;
//            }
//
//            return swapByteOrder(value);
//        }

        [[nodiscard]] bool isValid() const
        {
            return m_memory;
        }

        /**
         * Fetch the number of t-states since the Z80 was reset.
         *
         * The t-states are stored using a 32-bit unsigned value. This value will wrap when the number of t-states
         * overflows this storage capacity. For a 3.5MHz Z80 this is approximately 20 minutes 27 seconds.
         *
         * @return
         */
        inline std::uint32_t tStates() const
        {
            return m_tStates;
        }

        /**
         * For use when loading snapshots, etc. Generally speaking this should not be called.
         *
         * @param tStates
         */
        inline void setTStates(std::uint32_t tStates)
        {
            m_tStates = tStates;
        }

        // Register getters
        Registers & registers()
        {
            return m_registers;
        }

        [[nodiscard]] const Registers & registers() const
        {
            return m_registers;
        }

        //
        // register values in host byte order
        //
        
        /**
         * Retrieve the value of a register pair in host byte order.
         *
         * @param reg
         * @return
         */
        [[nodiscard]] UnsignedWord registerValue(Register16 reg) const;

        /**
         * Retrieve the value of a register pair in Z80 byte order.
         *
         * @param reg
         * @return
         */
        [[nodiscard]] UnsignedWord registerValueZ80(Register16 reg) const;

        /**
         * Retrieve a register value.
         *
         * Since the value is a single byte, there are no host/Z80 byte order issues.
         *
         * @param reg
         * @return
         */
        [[nodiscard]] UnsignedByte registerValue(Register8 reg) const;

        /* 16-bit registers */
        [[nodiscard]] inline UnsignedWord afRegisterValue() const
        {
            return m_registers.af;
        }

        [[nodiscard]] inline UnsignedWord bcRegisterValue() const
        {
            return m_registers.bc;
        }

        [[nodiscard]] inline UnsignedWord deRegisterValue() const
        {
            return m_registers.de;
        }

        [[nodiscard]] inline UnsignedWord hlRegisterValue() const
        {
            return m_registers.hl;
        }

        [[nodiscard]] inline UnsignedWord ixRegisterValue() const
        {
            return m_registers.ix;
        }

        [[nodiscard]] inline UnsignedWord iyRegisterValue() const
        {
            return m_registers.iy;
        }

        /**
         * The stack pointer in host byte order.
         */
        [[nodiscard]] inline UnsignedWord sp() const
        {
            return m_registers.sp;
        }

        /**
         * The stack pointer in host byte order.
         */
        [[nodiscard]] inline UnsignedWord stackPointer() const
        {
            return sp();
        }

        /**
         * The program counter in host byte order.
         */
        [[nodiscard]] inline UnsignedWord pc() const
        {
            return m_registers.pc;
        }

        /**
         * The program counter in host byte order.
         */
        [[nodiscard]] inline UnsignedWord programCounter() const
        {
            return pc();
        }

        //
        // shadow registers in host byte order
        //
        
        [[nodiscard]] inline UnsignedWord afShadowRegisterValue() const
        {
            return m_registers.afShadow;
        }

        [[nodiscard]] inline UnsignedWord bcShadowRegisterValue() const
        {
            return m_registers.bcShadow;
        }

        [[nodiscard]] inline UnsignedWord deShadowRegisterValue() const
        {
            return m_registers.deShadow;
        }

        [[nodiscard]] inline UnsignedWord hlShadowRegisterValue() const
        {
            return m_registers.hlShadow;
        }

        //
        // register values in Z80 byte order
        //
        
        [[nodiscard]] inline UnsignedWord afRegisterValueZ80() const
        {
            return registerValueZ80(Register16::AF);
        }

        [[nodiscard]] inline UnsignedWord bcRegisterValueZ80() const
        {
            return registerValueZ80(Register16::BC);
        }

        [[nodiscard]] inline UnsignedWord deRegisterValueZ80() const
        {
            return registerValueZ80(Register16::DE);
        }

        [[nodiscard]] inline UnsignedWord hlRegisterValueZ80() const
        {
            return registerValueZ80(Register16::HL);
        }

        [[nodiscard]] inline UnsignedWord ixRegisterValueZ80() const
        {
            return registerValueZ80(Register16::IX);
        }

        [[nodiscard]] inline UnsignedWord iyRegisterValueZ80() const
        {
            return registerValueZ80(Register16::IY);
        }

        /**
         * The stack pointer in Z80 byte order.
         */
        [[nodiscard]] inline UnsignedWord spZ80() const
        {
            return registerValueZ80(Register16::SP);
        }

        /**
         * The stack pointer in Z80 byte order.
         */
        [[nodiscard]] inline UnsignedWord stackPointerZ80() const
        {
            return spZ80();
        }

        /**
         * The program counter in Z80 byte order.
         */
        [[nodiscard]] inline UnsignedWord pcZ80() const
        {
            return registerValueZ80(Register16::PC);
        }

        /**
         * The program counter in Z80 byte order.
         */
        [[nodiscard]] inline UnsignedWord programCounterZ80() const
        {
            return pcZ80();
        }

        [[nodiscard]] inline UnsignedWord afShadowRegisterValueZ80() const
        {
            return registerValueZ80(Register16::AFShadow);
        }

        [[nodiscard]] inline UnsignedWord bcShadowRegisterValueZ80() const
        {
            return registerValueZ80(Register16::BCShadow);
        }

        [[nodiscard]] inline UnsignedWord deShadowRegisterValueZ80() const
        {
            return registerValueZ80(Register16::DEShadow);
        }

        [[nodiscard]] inline UnsignedWord hlShadowRegisterValueZ80() const
        {
            return registerValueZ80(Register16::HLShadow);
        }

        //
        // 8-bit registers
        //

        [[nodiscard]] inline UnsignedByte aRegisterValue() const
        {
            return m_registers.a;
        }

        [[nodiscard]] inline UnsignedByte fRegisterValue() const
        {
            return m_registers.f;
        }

        [[nodiscard]] inline UnsignedByte bRegisterValue() const
        {
            return m_registers.b;
        }

        [[nodiscard]] inline UnsignedByte cRegisterValue() const
        {
            return m_registers.c;
        }

        [[nodiscard]] inline UnsignedByte dRegisterValue() const
        {
            return m_registers.d;
        }

        [[nodiscard]] inline UnsignedByte eRegisterValue() const
        {
            return m_registers.e;
        }

        [[nodiscard]] inline UnsignedByte hRegisterValue() const
        {
            return m_registers.h;
        }

        [[nodiscard]] inline UnsignedByte lRegisterValue() const
        {
            return m_registers.l;
        }

        [[nodiscard]] inline UnsignedByte iRegisterValue() const
        {
            return m_registers.i;
        }

        [[nodiscard]] inline UnsignedByte rRegisterValue() const
        {
            return m_registers.r;
        }

        [[nodiscard]] inline UnsignedByte ixhRegisterValue() const
        {
            return m_registers.ixh;
        }

        [[nodiscard]] inline UnsignedByte ixlRegisterValue() const
        {
            return m_registers.ixl;
        }

        [[nodiscard]] inline UnsignedByte iyhRegisterValue() const
        {
            return m_registers.iyh;
        }

        [[nodiscard]] inline UnsignedByte iylRegisterValue() const
        {
            return m_registers.iyl;
        }

        [[nodiscard]] inline UnsignedByte aShadowRegisterValue() const
        {
            return m_registers.aShadow;
        }

        [[nodiscard]] inline UnsignedByte fShadowRegisterValue() const
        {
            return m_registers.fShadow;
        }

        [[nodiscard]] inline UnsignedByte bShadowRegisterValue() const
        {
            return m_registers.bShadow;
        }

        [[nodiscard]] inline UnsignedByte cShadowRegisterValue() const
        {
            return m_registers.cShadow;
        }

        [[nodiscard]] inline UnsignedByte dShadowRegisterValue() const
        {
            return m_registers.dShadow;
        }

        [[nodiscard]] inline UnsignedByte eShadowRegisterValue() const
        {
            return m_registers.eShadow;
        }

        [[nodiscard]] inline UnsignedByte hShadowRegisterValue() const
        {
            return m_registers.hShadow;
        }

        [[nodiscard]] inline UnsignedByte lShadowRegisterValue() const
        {
            return m_registers.lShadow;
        }

        inline void setInterruptMode(InterruptMode mode)
        {
            m_interruptMode = mode;
        }

        [[nodiscard]] inline InterruptMode interruptMode() const
        {
            return m_interruptMode;
        }

        inline void setIff1(bool iff)
        {
            m_iff1 = iff;
        }

        [[nodiscard]] inline bool iff1() const
        {
            return m_iff1;
        }

        inline void setIff2(bool iff)
        {
            m_iff2 = iff;
        }

        [[nodiscard]] inline bool iff2() const
        {
            return m_iff2;
        }

        //
        // Register setters
        //

        /**
         * Set the value of a register using an unsigned word in host byte order.
         *
         * The value will be converted to Z80 byte order if necessary before being stored.
         *
         * @param reg
         * @param value
         */
        void setRegisterValue(Register16 reg, UnsignedWord value);

        /**
         * Set the value of a register using an unsigned word in Z80 byte order.
         *
         * The value is assumed to already be in Z80 byte order an no conversion will be performed.
         *
         * @param reg
         * @param value
         */
        void setRegisterValueZ80(Register16 reg, UnsignedWord value);

        /**
         * Set the value of an 8-bit register.
         *
         * As a single byte, no byte order conversion is required.
         *
         * @param reg
         * @param value
         */
        void setRegisterValue(Register8 reg, UnsignedByte value);

        // set 16-bit register values using values in host byte order
        inline void setAf(UnsignedWord value)
        {
            m_registers.af = value;
        }

        inline void setBc(UnsignedWord value)
        {
            m_registers.bc = value;
        }

        inline void setDe(UnsignedWord value)
        {
            m_registers.de = value;
        }

        inline void setHl(UnsignedWord value)
        {
            m_registers.hl = value;
        }

        inline void setSp(UnsignedWord value)
        {
            m_registers.sp = value;
        }

        inline void setPc(UnsignedWord value)
        {
            m_registers.pc = value;
        }

        inline void setIx(UnsignedWord value)
        {
            m_registers.ix = value;
        }

        inline void setIy(UnsignedWord value)
        {
            m_registers.iy = value;
        }

        inline void setAfShadow(UnsignedWord value)
        {
            m_registers.afShadow = value;
        }

        inline void setBcShadow(UnsignedWord value)
        {
            m_registers.bcShadow = value;
        }

        inline void setDeShadow(UnsignedWord value)
        {
            m_registers.deShadow = value;
        }

        inline void setHlShadow(UnsignedWord value)
        {
            m_registers.hlShadow = value;
        }

        //
        // set 16-bit register pairs using values in Z80 byte order
        //

        inline void setAfZ80(UnsignedWord value)
        {
            m_registers.af = z80ToHostByteOrder(value);
        }

        inline void setBcZ80(UnsignedWord value)
        {
            m_registers.bc = z80ToHostByteOrder(value);
        }

        inline void setDeZ80(UnsignedWord value)
        {
            m_registers.de = z80ToHostByteOrder(value);
        }

        inline void setHlZ80(UnsignedWord value)
        {
            m_registers.hl = z80ToHostByteOrder(value);
        }

        inline void setSpZ80(UnsignedWord value)
        {
            m_registers.sp = z80ToHostByteOrder(value);
        }

        inline void setPcZ80(UnsignedWord value)
        {
            m_registers.pc = z80ToHostByteOrder(value);
        }

        inline void setIxZ80(UnsignedWord value)
        {
            m_registers.ix = z80ToHostByteOrder(value);
        }

        inline void setIyZ80(UnsignedWord value)
        {
            m_registers.iy = z80ToHostByteOrder(value);
        }

        inline void setAfShadowZ80(UnsignedWord value)
        {
            m_registers.afShadow = z80ToHostByteOrder(value);
        }

        inline void setBcShadowZ80(UnsignedWord value)
        {
            m_registers.bcShadow = z80ToHostByteOrder(value);
        }

        inline void setDeShadowZ80(UnsignedWord value)
        {
            m_registers.deShadow = z80ToHostByteOrder(value);
        }

        inline void setHlShadowZ80(UnsignedWord value)
        {
            m_registers.hlShadow = z80ToHostByteOrder(value);
        }

        //
        // 8-bit Registers
        //

        inline void setA(UnsignedByte value)
        {
            m_registers.a = value;
        }

        inline void setF(UnsignedByte value)
        {
            m_registers.f = value;
        }

        inline void setB(UnsignedByte value)
        {
            m_registers.b = value;
        }

        inline void setC(UnsignedByte value)
        {
            m_registers.c = value;
        }

        inline void setD(UnsignedByte value)
        {
            m_registers.d = value;
        }

        inline void setE(UnsignedByte value)
        {
            m_registers.e = value;
        }

        inline void setH(UnsignedByte value)
        {
            m_registers.h = value;
        }

        inline void setL(UnsignedByte value)
        {
            m_registers.l = value;
        }

        inline void setI(UnsignedByte value)
        {
            m_registers.i = value;
        }

        inline void setR(UnsignedByte value)
        {
            m_registers.r = value;
        }

        inline void setAShadow(UnsignedByte value)
        {
            m_registers.aShadow = value;
        }

        inline void setFShadow(UnsignedByte value)
        {
            m_registers.fShadow = value;
        }

        inline void setBShadow(UnsignedByte value)
        {
            m_registers.bShadow = value;
        }

        inline void setCShadow(UnsignedByte value)
        {
            m_registers.cShadow = value;
        }

        inline void setDShadow(UnsignedByte value)
        {
            m_registers.dShadow = value;
        }

        inline void setEShadow(UnsignedByte value)
        {
            m_registers.eShadow = value;
        }

        inline void setHShadow(UnsignedByte value)
        {
            m_registers.hShadow = value;
        }

        inline void setLShadow(UnsignedByte value)
        {
            m_registers.lShadow = value;
        }

        //
        // query flag states
        //
        /**
         * True if all flag bits from the mask are set, false otherwise.
         *
         * @tparam mask 
         * @return
         */
        template<UnsignedByte mask>
        [[nodiscard]] inline bool checkFlags() const
        {
            return mask == (m_registers.f & mask);
        }

        [[nodiscard]] inline bool sFlag() const
        {
            return checkFlags<Z80_FLAG_S_MASK>();
        }

        [[nodiscard]] inline bool zFlag() const
        {
            return checkFlags<Z80_FLAG_Z_MASK>();
        }

        [[nodiscard]] inline bool f5Flag() const
        {
            return checkFlags<Z80_FLAG_F5_MASK>();
        }

        [[nodiscard]] inline bool hFlag() const
        {
            return checkFlags<Z80_FLAG_H_MASK>();
        }

        [[nodiscard]] inline bool f3Flag() const
        {
            return checkFlags<Z80_FLAG_F3_MASK>();
        }

        [[nodiscard]] inline bool pFlag() const
        {
            return checkFlags<Z80_FLAG_P_MASK>();
        }

        [[nodiscard]] inline bool nFlag() const
        {
            return checkFlags<Z80_FLAG_N_MASK>();
        }

        [[nodiscard]] inline bool cFlag() const
        {
            return checkFlags<Z80_FLAG_C_MASK>();
        }

        template<UnsignedByte mask>
        [[nodiscard]] inline bool checkShadowFlags() const
        {
            return m_registers.fShadow & mask;
        }

        [[nodiscard]] inline bool sShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_S_MASK>();
        }

        [[nodiscard]] inline bool zShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_Z_MASK>();
        }

        [[nodiscard]] inline bool f5ShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_F3_MASK>();
        }

        [[nodiscard]] inline bool hShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_H_MASK>();
        }

        [[nodiscard]] inline bool f3ShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_F3_MASK>();
        }

        [[nodiscard]] inline bool pShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_P_MASK>();
        }

        [[nodiscard]] inline bool nShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_N_MASK>();
        }

        [[nodiscard]] inline bool cShadowFlag() const
        {
            return checkShadowFlags<Z80_FLAG_C_MASK>();
        }

        // convenience aliases
        [[nodiscard]] inline bool carryFlag() const
        {
            return cFlag();
        }

        [[nodiscard]] inline bool zeroFlag() const
        {
            return zFlag();
        }

        [[nodiscard]] inline bool signFlag() const
        {
            return sFlag();
        }

        [[nodiscard]] inline bool parityFlag() const
        {
            return pFlag();
        }

        [[nodiscard]] inline bool vFlag() const
        {
            return pFlag();
        }

        [[nodiscard]] inline bool overflowFlag() const
        {
            return pFlag();
        }

        [[nodiscard]] inline bool pvFlag() const
        {
            return pFlag();
        }

        [[nodiscard]] inline bool negationFlag() const
        {
            return nFlag();
        }

        [[nodiscard]] inline bool halfcarryFlag() const
        {
            return hFlag();
        }

        [[nodiscard]] inline bool carryShadowFlag() const
        {
            return cShadowFlag();
        }

        [[nodiscard]] inline bool zeroShadowFlag() const
        {
            return zShadowFlag();
        }

        [[nodiscard]] inline bool signShadowFlag() const
        {
            return sShadowFlag();
        }

        [[nodiscard]] inline bool parityShadowFlag() const
        {
            return pShadowFlag();
        }

        [[nodiscard]] inline bool vShadowFlag() const
        {
            return pShadowFlag();
        }

        [[nodiscard]] inline bool overflowShadowFlag() const
        {
            return pShadowFlag();
        }

        [[nodiscard]] inline bool pvShadowFlag() const
        {
            return pShadowFlag();
        }

        [[nodiscard]] inline bool negationShadowFlag() const
        {
            return nShadowFlag();
        }

        [[nodiscard]] inline bool halfcarryShadowFlag() const
        {
            return hShadowFlag();
        }

        /**
         * Fetch an 8-bit value from the Z80 memory.
         *
         * @param addr The address of the value to fetch. The address is given in host byte order.
         *
         * @return
         */
        [[nodiscard]] inline UnsignedByte peekUnsigned(MemoryType::Address addr) const
        {
            if (addr < 0 || addr >= memorySize()) {
                return 0;
            }

            return m_memory->readByte(addr);
        }

        /**
         * Fetch a 16-bit value from the Z80 memory.
         *
         * The value is converted to host byte order.
         *
         * @param addr The address of the value to fetch. The address is given in host byte order.
         *
         * @return
         */
        [[nodiscard]] UnsignedWord peekUnsignedHostWord(MemoryType::Address addr) const;

        /**
         * Fetch a 16-bit value from the Z80 memory.
         *
         * The value is return in Z80 byte order.
         *
         * @param addr The address of the value to fetch. The address is given in host byte order.
         *
         * @return
         */
        [[nodiscard]] UnsignedWord peekUnsignedZ80Word(MemoryType::Address addr) const;

        inline SignedByte peekSigned(MemoryType::Address addr) const
        {
            return static_cast<SignedByte>(peekUnsigned(addr));
        }

        /**
         * Write an 8-bit value to a memory address.
         *
         * @param addr The address to write the 8-bit value. The address is given in host byte order.
         * @param value
         */
        inline void pokeUnsigned(MemoryType::Address addr, UnsignedByte value);

        /**
         * Write a 16-bit value to the Z80 memory.
         *
         * @param addr The address to write the first byte of the 16-bit value. The address is given in host byte order.
         * @param value The 16-bit value to write, in host byte order.
         */
        void pokeHostWord(MemoryType::Address addr, UnsignedWord value);

        /**
         * Write a 16-bit value to the Z80 memory.
         *
         * @param addr The address to write the first byte of the 16-bit value. The address is given in host byte order.
         * @param value The 16-bit value to write, in Z80 byte order.
         */
        void pokeZ80Word(MemoryType::Address addr, UnsignedWord value);

        void reset();

        /**
         * Connect an IO device to the Z80.
         *
         * When an IN or OUT instruction is encountered, the device will be asked if it is interested in responding on
         * the port specified in the instruction, and if it is it will be provided with the byte written (for OUT
         * instructions, or asked to provide a byte (for IN instructions).
         *
         * The device is not owned by the Z80 and the caller is responsible for its disposal and for ensuring the Z80
         * does not retain references to devices that have been destroyed.
         *
         * @param device
         * @return
         */
        bool connectIODevice(IODevice * device);

        /**
         * Disconnect a previously connected IO device from the Z80.
         *
         * @param device
         * @return
         */
        void disconnectIODevice(IODevice * device);

        /**
         * Trigger a non-maskable interrupt (NMI).
         */
        void nmi();

        /**
         * Trigger a maskable interrupt.
         *
         * If interrupts are currently being handled (IFF1 == true) then the interrupt will be triggered after the next
         * instruction has finished executing.
         *
         * @param data
         */
        void interrupt(UnsignedByte data = 0x00);

        /**
         * Execute a single instruction.
         *
         * The bytes for the instruction machine code must be sufficient to fully represent a single instruction. The
         * most bytes any single instruction requires is 4. For REPL/interpreter-style scenarios, you can set doPc to
         * false to suppress emulation of the program counter. For example, if you are executing instructions assembled
         * from input provided by the user and not stored somewhere in the Z80's memory, changing the PC probably has
         * little meaning.
         *
         * @param instruction The opcode bytes for the instruction.
         * @param doPc Whether or not the PC should be altered by executing the instruction.
         *
         * @return
         */
        virtual InstructionCost execute(const UnsignedByte * instruction, bool doPc);

        /**
         * Run the fetch-execute cycle on the Z80.
         *
         * The Z80 fetches a single instruction from memory at the address stored in the program counter, executes the
         * instruction, and checks for and handles any pending interrupt.
         *
         * @return The number of t-states consumed by the cycle. This includes the t-states consumed by the interrupt if
         * one was pending and handled.
         */
        virtual int fetchExecuteCycle();

    protected:
        virtual void handleNmi();

        virtual int handleInterrupt();

        // doPc is altered to be false if the PC has been altered during instruction execution and should therefore not
        // be incremented by the caller
        InstructionCost executePlainInstruction(const UnsignedByte * instruction, bool * doPc);
        InstructionCost executeCbInstruction(const UnsignedByte * instruction);
        InstructionCost executeEdInstruction(const UnsignedByte * instruction, bool * doPc);
        InstructionCost executeDdOrFdInstruction(UnsignedWord & reg, const UnsignedByte * instruction, bool * doPc);
        InstructionCost executeDdcbOrFdcbInstruction(UnsignedWord & reg, const UnsignedByte * instruction);

    private:
        static const int PlainOpcodeTStates[256];
        static const std::uint8_t CbOpcodeTStates[256];
        static const int DdOrFdOpcodeTStates[256];
        static const std::uint8_t EdOpcodeTStates[256];
        static const std::uint8_t DdCbOrFdCbOpcodeTStates[256];

        static const std::uint8_t PlainOpcodeSizes[256];
        static const std::uint8_t DdOrFdOpcodeSizes[256];
        static const std::uint8_t EdOpcodeSizes[256];

        Registers m_registers;

        // memory
        Memory * m_memory;

        // interrupt handling
        std::uint32_t m_tStates;
        bool m_iff1;
        bool m_iff2;
        InterruptMode m_interruptMode;
        bool m_nmiPending;
        bool m_interruptRequested;
        UnsignedByte m_interruptData;
        bool m_delayInterruptOneInstruction;

        // set when a HALT instruction is executed; cleared when an interrupt occurs or the CPU is reset
        bool m_halted;
        
        unsigned long long m_clockSpeed;    // clock speed in Hz
        std::set<IODevice *> m_ioDevices;

#if (!defined(NDEBUG))
    public:
        // write details about the current state of the CPU
        void dumpState(std::ostream & out = std::cout) const;

        // write details about the N most recently executed instructions
        void dumpExecutionHistory(int entries, std::ostream & out = std::cout) const;

    private:
        // a ring buffer with the 10000 most recently executed instructions
        ExecutionHistory<10000> m_executionHistory;
#endif
    };
}

#endif // Z80_Z80_H
