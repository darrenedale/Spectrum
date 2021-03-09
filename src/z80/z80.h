#ifndef Z80_Z80_H
#define Z80_Z80_H

#include <cassert>
#include <cstdint>
#include <bit>
#include <set>

#include "../cpu.h"
#include "registers.h"

#define Z80_DEFAULT_RAM_SIZE = 65536
#define Z80_MHZ(n) (n * 1000000.0)

#define Z80_FLAG_C_BIT 0
#define Z80_FLAG_Z_BIT 6
#define Z80_FLAG_P_BIT 2
#define Z80_FLAG_S_BIT 7
#define Z80_FLAG_N_BIT 1
#define Z80_FLAG_H_BIT 4
#define Z80_FLAG_F3_BIT 3
#define Z80_FLAG_F5_BIT 5

#define Z80_FLAG_C_MASK (1 << Z80_FLAG_C_BIT)
#define Z80_FLAG_Z_MASK (1 << Z80_FLAG_Z_BIT)
#define Z80_FLAG_P_MASK (1 << Z80_FLAG_P_BIT)
#define Z80_FLAG_S_MASK (1 << Z80_FLAG_S_BIT)
#define Z80_FLAG_N_MASK (1 << Z80_FLAG_N_BIT)
#define Z80_FLAG_H_MASK (1 << Z80_FLAG_H_BIT)
#define Z80_FLAG_F3_MASK (1 << Z80_FLAG_F3_BIT)
#define Z80_FLAG_F5_MASK (1 << Z80_FLAG_F5_BIT)

namespace Z80 {
    class Z80IODevice;

    class Z80
            : public Cpu {
    public:
        typedef std::uint8_t UnsignedByte;
        typedef std::uint16_t UnsignedWord;
        typedef std::int8_t SignedByte;
        typedef std::int16_t SignedWord;

        enum class InterruptMode : std::uint8_t
        {
            IM0 = 0,
            IM1,
            IM2,
        };

        enum class Register16 {
            AF, BC, DE, HL,
            IX, IY,
            SP, PC,
            AFShadow, BCShadow, DEShadow, HLShadow
        };

        enum class Register8 {
            A, F, B, C, D, E, H, L,
            IXH, IXL, IYH, IYL,
            I, R,
            AShadow, FShadow, BShadow, CShadow, DShadow, EShadow, HShadow, LShadow
        };

        static constexpr const std::endian Z80ByteOrder = std::endian::little;
        static constexpr const std::endian HostByteOrder = std::endian::native;

        Z80(std::uint8_t *mem, int memSize);

        ~Z80() override;

        static inline UnsignedWord swapByteOrder(UnsignedWord v) {
            return (((v & 0xff00) >> 8) & 0x00ff) | (((v & 0x00ff) << 8) & 0xff00);
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

        static inline bool isEvenParity(UnsignedByte v);

        static inline bool isEvenParity(UnsignedWord v);

        bool isValid() const
        {
            return m_ram;
        }

        /* Register getters */
        Registers & registers()
        {
            return m_registers;
        }

        const Registers & registers() const
        {
            return m_registers;
        }

        /**
         * Retrieve the value of a register pair in host byte order.
         *
         * @param reg
         * @return
         */
        UnsignedWord registerValue(Register16 reg) const;

        /**
         * Retrieve the value of a register pair in Z80 byte order.
         *
         * @param reg
         * @return
         */
        UnsignedWord registerValueZ80(Register16 reg) const;

        /**
         * Retrieve a register value.
         *
         * Since the value is a single byte, there are no host/Z80 byte order issues.
         *
         * @param reg
         * @return
         */
        UnsignedByte registerValue(Register8 reg) const;

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
        // the values are converted to Z80 byte order before they are set
        void setAf(UnsignedWord value) {
            setRegisterValue(Register16::AF, value);
        }

        void setBc(UnsignedWord value) {
            setRegisterValue(Register16::BC, value);
        }

        void setDe(UnsignedWord value) {
            setRegisterValue(Register16::DE, value);
        }

        void setHl(UnsignedWord value) {
            setRegisterValue(Register16::HL, value);
        }

        void setSp(UnsignedWord value) {
            setRegisterValue(Register16::SP, value);
        }

        void setPc(UnsignedWord value) {
            setRegisterValue(Register16::PC, value);
        }

        void setIx(UnsignedWord value) {
            setRegisterValue(Register16::IX, value);
        }

        void setIy(UnsignedWord value) {
            setRegisterValue(Register16::IY, value);
        }

        void setAfShadow(UnsignedWord value) {
            setRegisterValue(Register16::AFShadow, value);
        }

        void setBcShadow(UnsignedWord value) {
            setRegisterValue(Register16::BCShadow, value);
        }

        void setDeShadow(UnsignedWord value) {
            setRegisterValue(Register16::DEShadow, value);
        }

        void setHlShadow(UnsignedWord value) {
            setRegisterValue(Register16::HLShadow, value);
        }

        // set 16-bit register pairs using values in Z80 byte order
        void setAfZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::AF, value);
        }

        void setBcZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::BC, value);
        }

        void setDeZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::DE, value);
        }

        void setHlZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::HL, value);
        }

        void setSpZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::SP, value);
        }

        void setPcZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::PC, value);
        }

        void setIxZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::IX, value);
        }

        void setIyZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::IY, value);
        }

        void setAfShadowZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::AFShadow, value);
        }

        void setBcShadowZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::BCShadow, value);
        }

        void setDeShadowZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::DEShadow, value);
        }

        void setHlShadowZ80(UnsignedWord value) {
            setRegisterValueZ80(Register16::HLShadow, value);
        }

        /* 8-bit Registers */
        void setA(UnsignedByte value) {
            setRegisterValue(Register8::A, value);
        }

        void setF(UnsignedByte value) {
            setRegisterValue(Register8::F, value);
        }

        void setB(UnsignedByte value) {
            setRegisterValue(Register8::B, value);
        }

        void setC(UnsignedByte value) {
            setRegisterValue(Register8::C, value);
        }

        void setD(UnsignedByte value) {
            setRegisterValue(Register8::D, value);
        }

        void setE(UnsignedByte value) {
            setRegisterValue(Register8::E, value);
        }

        void setH(UnsignedByte value) {
            setRegisterValue(Register8::H, value);
        }

        void setL(UnsignedByte value) {
            setRegisterValue(Register8::L, value);
        }

        void setI(UnsignedByte value) {
            setRegisterValue(Register8::I, value);
        }

        void setR(UnsignedByte value) {
            setRegisterValue(Register8::R, value);
        }

        void setAShadow(UnsignedByte value) {
            setRegisterValue(Register8::AShadow, value);
        }

        void setFShadow(UnsignedByte value) {
            setRegisterValue(Register8::FShadow, value);
        }

        void setBShadow(UnsignedByte value) {
            setRegisterValue(Register8::BShadow, value);
        }

        void setCShadow(UnsignedByte value) {
            setRegisterValue(Register8::CShadow, value);
        }

        void setDShadow(UnsignedByte value) {
            setRegisterValue(Register8::DShadow, value);
        }

        void setEShadow(UnsignedByte value) {
            setRegisterValue(Register8::EShadow, value);
        }

        void setHShadow(UnsignedByte value) {
            setRegisterValue(Register8::HShadow, value);
        }

        void setLShadow(UnsignedByte value) {
            setRegisterValue(Register8::LShadow, value);
        }

        inline int ramSize() const {
            return m_ramSize;
        }

        /* memory access methods - values provided in host byte order */
        inline Z80::UnsignedByte *memory() const {
            return m_ram;
        }

        inline bool cFlag() const {
            return m_registers.f & Z80_FLAG_C_MASK;
        }

        inline bool zFlag() const {
            return m_registers.f & Z80_FLAG_Z_MASK;
        }

        inline bool sFlag() const {
            return m_registers.f & Z80_FLAG_S_MASK;
        }

        inline bool pFlag() const {
            return m_registers.f & Z80_FLAG_P_MASK;
        }

        inline bool nFlag() const {
            return m_registers.f & Z80_FLAG_N_MASK;
        }

        inline bool hFlag() const {
            return m_registers.f & Z80_FLAG_H_MASK;
        }

        inline bool cShadowFlag() const {
            return  m_registers.fShadow & Z80_FLAG_C_MASK;
        }

        inline bool zShadowFlag() const {
            return  m_registers.fShadow & Z80_FLAG_Z_MASK;
        }

        inline bool sShadowFlag() const {
            return  m_registers.fShadow & Z80_FLAG_S_MASK;
        }

        inline bool pShadowFlag() const {
            return  m_registers.fShadow & Z80_FLAG_P_MASK;
        }

        inline bool nShadowFlag() const {
            return  m_registers.fShadow & Z80_FLAG_N_MASK;
        }

        inline bool hShadowFlag() const {
            return  m_registers.fShadow & Z80_FLAG_H_MASK;
        }

        inline bool carryFlag() const {
            return cFlag();
        }

        inline bool zeroFlag() const {
            return zFlag();
        }

        inline bool signFlag() const {
            return sFlag();
        }

        inline bool parityFlag() const {
            return pFlag();
        }

        inline bool vFlag() const {
            return pFlag();
        }

        inline bool overflowFlag() const {
            return pFlag();
        }

        inline bool pvFlag() const {
            return pFlag();
        }

        inline bool negationFlag() const {
            return nFlag();
        }

        inline bool halfcarryFlag() const {
            return hFlag();
        }

        inline bool carryShadowFlag() const {
            return cShadowFlag();
        }

        inline bool zeroShadowFlag() const {
            return zShadowFlag();
        }

        inline bool signShadowFlag() const {
            return sShadowFlag();
        }

        inline bool parityShadowFlag() const {
            return pShadowFlag();
        }

        inline bool vShadowFlag() const {
            return pShadowFlag();
        }

        inline bool overflowShadowFlag() const {
            return pShadowFlag();
        }

        inline bool pvShadowFlag() const {
            return pShadowFlag();
        }

        inline bool negationShadowFlag() const {
            return nShadowFlag();
        }

        inline bool halfcarryShadowFlag() const {
            return hShadowFlag();
        }

        /**
         * Fetch an 8-bit value from the Z80 memory.
         *
         * @param addr The address of the value to fetch. The address is given in host byte order.
         *
         * @return
         */
        inline UnsignedByte peekUnsigned(int addr) const {
            if (addr < 0 || addr >= m_ramSize) {
                return 0;
            }

            return m_ram[addr];
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
        UnsignedWord peekUnsignedWord(int addr) const;

        /**
         * Fetch a 16-bit value from the Z80 memory.
         *
         * The value is return in Z80 byte order.
         *
         * @param addr The address of the value to fetch. The address is given in host byte order.
         *
         * @return
         */
        UnsignedWord peekUnsignedWordZ80(int addr) const;

        inline SignedByte peekSigned(int addr) const {
            return SignedByte(peekUnsigned(addr));
        }

        /**
         * Write an 8-bit value to a memory address.
         *
         * @param addr The address to write the 8-bit value. The address is given in host byte order.
         * @param value
         */
        inline void pokeUnsigned(int addr, UnsignedByte value) {
            if (addr < 0 || addr > m_ramSize) {
                return;
            }

            m_ram[addr] = value;
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

        /**
         * Write a 16-bit value to the Z80 memory.
         *
         * @param addr The address to write the first byte of the 16-bit value. The address is given in host byte order.
         * @param value The 16-bit value to write, in Z80 byte order.
         */
        void pokeUnsignedWordZ80(int addr, UnsignedWord value);

        void reset();

        void start();

        bool connectIODevice(Z80IODevice * device);
        void disconnectIODevice(Z80IODevice * device);

        void interrupt(UnsignedWord data = 0x0000);
        void nmi();

        bool load(unsigned char *data, int addr, int length);

        bool execute(const UnsignedByte *instruction, bool doPc = true, int *tStates = 0, int *size = 0);

        /* fetches and executes a single instruction, returns the cycle cost */
        int fetchExecuteCycle();

    protected:
        /* doPc is altered to be false if the instruction is a jump that is taken;
         * cycles is filled with clock cycles consumed; size is filled with byte
         * size of instruction and operands; doPc is set to false if the
         * instruction has modified the PC. returns true if execution of
         * instruction was successful, false otherwise */
        bool executePlainInstruction(const UnsignedByte *instruction, bool *doPc = nullptr, int *cycles = nullptr,
                                     int *size = nullptr);

        bool executeCbInstruction(const UnsignedByte *instruction, int *cycles = nullptr, int *size = nullptr);

        bool executeEdInstruction(const UnsignedByte *instruction, bool *doPc = nullptr, int *cycles = nullptr,
                                  int *size = nullptr);

        bool executeDdOrFdInstruction(UnsignedWord &reg, const UnsignedByte *instruction, bool *doPc = nullptr,
                                      int *cycles = nullptr, int *size = nullptr);

        bool executeDdcbOrFdcbInstruction(UnsignedWord &reg, const UnsignedByte *instruction, int *tStates = nullptr,
                                          int *size = nullptr);
//		bool executeFdInstruction( const UnsignedByte * instruction, bool * doPc = 0, int * cycles = 0, int * size = 0 );


    private:
        /* NEVER call this method more than once. most subclasses will never need
         * to call it as the Z80 constructor makes sure the call is made */
        void init();

        Registers m_registers;

        // memory
        Z80::UnsignedByte *m_ram;
        int m_ramSize;
        bool m_iff1;
        bool m_iff2;
        InterruptMode m_interruptMode;
        bool m_nmiPending;
        bool m_interruptRequested;
        Z80::UnsignedWord m_interruptData;

        unsigned int m_plain_opcode_cycles[256];

        // 16-bit opcodes starting 0xdd and 0xfd use the IX and IY registers respectively for identical instructions, so
        // their opcode costs are identical
        unsigned int m_ddorfd_cb_opcode_cycles[256];
        unsigned int m_ddorfd_opcode_cycles[256];
        unsigned int m_fd_opcode_cycles[256];
        unsigned int m_ed_opcode_cycles[256];
        unsigned int m_cb_opcode_cycles[256];
        unsigned int m_plain_opcode_size[256];
        static const unsigned int DdOrFdOpcodeSize[256];
        unsigned int m_fd_opcode_size[256];
        unsigned int m_ed_opcode_size[256];
        unsigned long long m_clockSpeed;    /* clock speed in Hz */
        std::set<Z80IODevice *> m_ioDevices;
    };
}

#endif // Z80_Z80_H
