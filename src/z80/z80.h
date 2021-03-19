#ifndef Z80_Z80_H
#define Z80_Z80_H

#include <cassert>
#include <cstdint>
#include <bit>
#include <set>

#include "../cpu.h"
#include "registers.h"
#include "types.h"

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
    : public Cpu {
    public:
        using UnsignedByte = ::Z80::UnsignedByte;
        using UnsignedWord = ::Z80::UnsignedWord;
        using SignedByte = ::Z80::SignedByte;
        using SignedWord = ::Z80::SignedWord;
        using Register8 = ::Z80::Register8;
        using Register16 = ::Z80::Register16;

        enum class InterruptMode : std::uint8_t
        {
            IM0 = 0,
            IM1,
            IM2,
        };

        static constexpr const std::endian Z80ByteOrder = std::endian::little;
        static constexpr const std::endian HostByteOrder = std::endian::native;

        Z80(UnsignedByte * memory, int memorySize);

        ~Z80() override;

        static inline UnsignedWord swapByteOrder(UnsignedWord value) {
            return (((value & 0xff00) >> 8) & 0x00ff) | (((value & 0x00ff) << 8) & 0xff00);
        }

        static inline UnsignedWord z80ToHostByteOrder(UnsignedWord v)
        {
            if (Z80ByteOrder == HostByteOrder) {
                return v;
            }

            return swapByteOrder(v);
        }

        static inline UnsignedWord hostToZ80ByteOrder(UnsignedWord v)
        {
            if (Z80ByteOrder == HostByteOrder) {
                return v;
            }

            return swapByteOrder(v);
        }

        [[nodiscard]] bool isValid() const
        {
            return m_memory;
        }

        [[nodiscard]] inline int ramSize() const {
            return m_memorySize;
        }

        [[nodiscard]] inline UnsignedByte * memory() const {
            return m_memory;
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
        inline UnsignedWord afRegisterValue() const {
            return registerValue(Register16::AF);
        }

        inline UnsignedWord bcRegisterValue() const {
            return registerValue(Register16::BC);
        }

        inline UnsignedWord deRegisterValue() const {
            return registerValue(Register16::DE);
        }

        inline UnsignedWord hlRegisterValue() const {
            return registerValue(Register16::HL);
        }

        inline UnsignedWord ixRegisterValue() const {
            return registerValue(Register16::IX);
        }

        inline UnsignedWord iyRegisterValue() const {
            return registerValue(Register16::IY);
        }

        /* stack pointer */
        inline UnsignedWord sp() const {
            return registerValue(Register16::SP);
        }

        inline UnsignedWord stackPointer() const {
            return sp();
        }

        /* program counter */
        inline UnsignedWord pc() const {
            return registerValue(Register16::PC);
        }

        inline UnsignedWord programCounter() const {
            return pc();
        }

        inline UnsignedWord afShadowRegisterValue() const {
            return registerValue(Register16::AFShadow);
        }

        inline UnsignedWord bcShadowRegisterValue() const {
            return registerValue(Register16::BCShadow);
        }

        inline UnsignedWord deShadowRegisterValue() const {
            return registerValue(Register16::DEShadow);
        }

        inline UnsignedWord hlShadowRegisterValue() const {
            return registerValue(Register16::HLShadow);
        }

        inline UnsignedWord afRegisterValueZ80() const {
            return registerValueZ80(Register16::AF);
        }

        inline UnsignedWord bcRegisterValueZ80() const {
            return registerValueZ80(Register16::BC);
        }

        inline UnsignedWord deRegisterValueZ80() const {
            return registerValueZ80(Register16::DE);
        }

        inline UnsignedWord hlRegisterValueZ80() const {
            return registerValueZ80(Register16::HL);
        }

        inline UnsignedWord ixRegisterValueZ80() const {
            return registerValueZ80(Register16::IX);
        }

        inline UnsignedWord iyRegisterValueZ80() const {
            return registerValueZ80(Register16::IY);
        }

        /* stack pointer */
        inline UnsignedWord spZ80() const {
            return registerValueZ80(Register16::SP);
        }

        inline UnsignedWord stackPointerZ80() const {
            return spZ80();
        }

        /* program counter */
        inline UnsignedWord pcZ80() const {
            return registerValueZ80(Register16::PC);
        }

        inline UnsignedWord programCounterZ80() const {
            return pcZ80();
        }

        inline UnsignedWord afShadowRegisterValueZ80() const {
            return registerValueZ80(Register16::AFShadow);
        }

        inline UnsignedWord bcShadowRegisterValueZ80() const {
            return registerValueZ80(Register16::BCShadow);
        }

        inline UnsignedWord deShadowRegisterValueZ80() const {
            return registerValueZ80(Register16::DEShadow);
        }

        inline UnsignedWord hlShadowRegisterValueZ80() const {
            return registerValueZ80(Register16::HLShadow);
        }

        /* 8-bit registers */
        inline UnsignedByte aRegisterValue() const {

            return registerValue(Register8::A);
        }

        inline UnsignedByte fRegisterValue() const {
            return registerValue(Register8::F);
        }

        inline UnsignedByte bRegisterValue() const {
            return registerValue(Register8::B);
        }

        inline UnsignedByte cRegisterValue() const {
            return registerValue(Register8::C);
        }

        inline UnsignedByte dRegisterValue() const {
            return registerValue(Register8::D);
        }

        inline UnsignedByte eRegisterValue() const {
            return registerValue(Register8::E);
        }

        inline UnsignedByte hRegisterValue() const {
            return registerValue(Register8::H);
        }

        inline UnsignedByte lRegisterValue() const {
            return registerValue(Register8::L);
        }

        inline UnsignedByte iRegisterValue() const {
            return registerValue(Register8::L);
        }

        inline UnsignedByte rRegisterValue() const {
            return registerValue(Register8::R);
        }

        inline UnsignedByte ixhRegisterValue() const {
            return registerValue(Register8::IXH);
        }

        inline UnsignedByte ixlRegisterValue() const {
            return registerValue(Register8::IXL);
        }

        inline UnsignedByte iyhRegisterValue() const {
            return registerValue(Register8::IYH);
        }

        inline UnsignedByte iylRegisterValue() const {
            return registerValue(Register8::IYL);
        }

        inline UnsignedByte aShadowRegisterValue() const {
            return registerValue(Register8::AShadow);
        }

        inline UnsignedByte fShadowRegisterValue() const {
            return registerValue(Register8::FShadow);
        }

        inline UnsignedByte bShadowRegisterValue() const {
            return registerValue(Register8::BShadow);
        }

        inline UnsignedByte cShadowRegisterValue() const {
            return registerValue(Register8::CShadow);
        }

        inline UnsignedByte dShadowRegisterValue() const {
            return registerValue(Register8::DShadow);
        }

        inline UnsignedByte eShadowRegisterValue() const {
            return registerValue(Register8::EShadow);
        }

        inline UnsignedByte hShadowRegisterValue() const {
            return registerValue(Register8::HShadow);
        }

        inline UnsignedByte lShadowRegisterValue() const {
            return registerValue(Register8::LShadow);
        }

        inline void setInterruptMode(InterruptMode mode)
        {
            m_interruptMode = mode;
        }

        inline InterruptMode interruptMode() const
        {
            return m_interruptMode;
        }

        inline void setIff1(bool iff)
        {
            m_iff1 = iff;
        }

        inline bool iff1() const {
            return m_iff1;
        }

        inline void setIff2(bool iff)
        {
            m_iff2 = iff;
        }

        inline bool iff2() const {
            return m_iff2;
        }

        /* Register setters */
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

        // set 16-bit register pairs using values in Z80 byte order
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

        /* 8-bit Registers */
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
        template <UnsignedByte mask>
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

        template <UnsignedByte mask>
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
        [[nodiscard]] inline UnsignedByte peekUnsigned(int addr) const
        {
            if (addr < 0 || addr >= m_memorySize) {
                return 0;
            }

            return m_memory[addr];
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
        [[nodiscard]] UnsignedWord peekUnsignedWord(int addr) const;

        /**
         * Fetch a 16-bit value from the Z80 memory.
         *
         * The value is return in Z80 byte order.
         *
         * @param addr The address of the value to fetch. The address is given in host byte order.
         *
         * @return
         */
        [[nodiscard]] UnsignedWord peekUnsignedWordZ80(int addr) const;

        inline SignedByte peekSigned(int addr) const {
            return static_cast<SignedByte>(peekUnsigned(addr));
        }

        /**
         * Write an 8-bit value to a memory address.
         *
         * @param addr The address to write the 8-bit value. The address is given in host byte order.
         * @param value
         */
        inline void pokeUnsigned(int addr, UnsignedByte value) {
            if (addr < 0 || addr > m_memorySize) {
                return;
            }

            m_memory[addr] = value;
        }

        /**
         * Write a 16-bit value to the Z80 memory.
         *
         * @param addr The address to write the first byte of the 16-bit value. The address is given in host byte order.
         * @param value The 16-bit value to write, in host byte order.
         */
        void pokeHostWord(int addr, UnsignedWord value);

        /**
         * Write a 16-bit value to the Z80 memory.
         *
         * @param addr The address to write the first byte of the 16-bit value. The address is given in host byte order.
         * @param value The 16-bit value to write, in Z80 byte order.
         */
        void pokeZ80Word(int addr, UnsignedWord value);

        void reset();

        bool connectIODevice(IODevice * device);
        void disconnectIODevice(IODevice * device);

        void nmi();
        void interrupt(UnsignedByte data = 0x00);

        virtual void execute(const UnsignedByte *instruction, bool doPc = true, int *tStates = 0, int *size = 0);

        // fetches and executes a single instruction, returns the number of t-states
        virtual int fetchExecuteCycle();

    protected:
        virtual void handleNmi();
        virtual int handleInterrupt();

        // doPc is altered to be false if the instruction is a jump that is taken; tStates is filled with clock cycles
        // consumed; size is filled with byte size of instruction and operands; doPc is set to false if the instruction
        // has modified the PC. returns true if execution of instruction was successful, false otherwise
        void executePlainInstruction(const UnsignedByte *instruction, bool *doPc = nullptr, int *tStates = nullptr,
                                     int *size = nullptr);

        void executeCbInstruction(const UnsignedByte *instruction, int *tStates = nullptr, int *size = nullptr);

        void executeEdInstruction(const UnsignedByte *instruction, bool *doPc = nullptr, int *tStates = nullptr,
                                  int *size = nullptr);

        void executeDdOrFdInstruction(UnsignedWord &reg, const UnsignedByte *instruction, bool *doPc = nullptr,
                                      int *tStates = nullptr, int *size = nullptr);

        void executeDdcbOrFdcbInstruction(UnsignedWord &reg, const UnsignedByte *instruction, int *tStates = nullptr,
                                          int *size = nullptr);

    private:
        static const int PlainOpcodeTStates[256];
        static const int CbOpcodeTStates[256];
        static const int DdOrFdOpcodeTStates[256];
        static const int EdOpcodeTStates[256];
        static const int DdCbOrFdCbOpcodeTStates[256];

        static const int PlainOpcodeSizes[256];
        static const int DdOrFdOpcodeSizes[256];
        static const int EdOpcodeSizes[256];

        Registers m_registers;

        // memory
        UnsignedByte * m_memory;
        int m_memorySize;

        // interrupt handling
        bool m_iff1;
        bool m_iff2;
        InterruptMode m_interruptMode;
        bool m_nmiPending;
        bool m_interruptRequested;
        UnsignedByte m_interruptData;

        unsigned long long m_clockSpeed;    /* clock speed in Hz */
        std::set<IODevice *> m_ioDevices;
    };
}

#endif // Z80_Z80_H
