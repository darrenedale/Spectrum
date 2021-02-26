#ifndef Z80_H
#define Z80_H

#include <cstdint>

#include "../cpu.h"

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

class Z80IODevice;

class Z80
: public Cpu
{
public:
    typedef std::uint8_t UnsignedByte;
    typedef std::uint16_t UnsignedWord;
    typedef std::int8_t SignedByte;
    typedef std::int16_t SignedWord;

    enum class Register16
    {
        AF, BC, DE, HL,
        IX, IY,
        SP, PC,
        AFShadow, BCShadow, DEShadow, HLShadow
    };

    enum class Register8
    {
        A, F, B, C, D, E, H, L,
        IXH, IXL, IYH, IYL,
        I, R,
        AShadow, FShadow, BShadow, CShadow, DShadow, EShadow, HShadow, LShadow
    };

    enum ByteOrder
    {
        BigEndian,
        LittleEndian
    };

    Z80(std::uint8_t * mem, int memSize);

    ~Z80() override;

    static inline ByteOrder z80ByteOrder()
    {
        return LittleEndian;
    }

    static inline ByteOrder hostByteOrder()
    {
        static int i = 1;
        return (0 == (unsigned char *) &i ? LittleEndian : BigEndian);
    }

    static inline short swapByteOrder(short v)
    {
        return (((v & 0xff00) >> 8) & 0x00ff) | (((v & 0x00ff) << 8) & 0xff00);
    }

    static inline short z80ToHostByteOrder(short v)
    {
        return (z80ByteOrder() == hostByteOrder() ? v : swapByteOrder(v));
    }

    static inline short hostToZ80ByteOrder(short v)
    {
        if (z80ByteOrder() == hostByteOrder()) {
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

    /* Register access methods - values provided in host byte order */
    /* Register getters */
    UnsignedWord registerValue(Register16 reg) const;

    UnsignedByte registerValue(Register8 reg) const;

    /* 16-bit registers */
    inline UnsignedWord afRegisterValue() const
    {
        return registerValue(Register16::AF);
    }

    inline UnsignedWord bcRegisterValue() const
    {
        return registerValue(Register16::BC);
    }

    inline UnsignedWord deRegisterValue() const
    {
        return registerValue(Register16::DE);
    }

    inline UnsignedWord hlRegisterValue() const
    {
        return registerValue(Register16::HL);
    }

    inline UnsignedWord ixRegisterValue() const
    {
        return registerValue(Register16::IX);
    }

    inline UnsignedWord iyRegisterValue() const
    {
        return registerValue(Register16::IY);
    }

    /* stack pointer */
    inline UnsignedWord sp() const
    {
        return registerValue(Register16::SP);
    }

    inline UnsignedWord stackPointer() const
    {
        return sp();
    }

    /* program counter */
    inline UnsignedWord pc() const
    {
        return registerValue(Register16::PC);
    }

    inline UnsignedWord programCounter() const
    {
        return pc();
    }

    inline UnsignedWord afShadowRegisterValue() const
    {
        return registerValue(Register16::AFShadow);
    }

    inline UnsignedWord bcShadowRegisterValue() const
    {
        return registerValue(Register16::BCShadow);
    }

    inline UnsignedWord deShadowRegisterValue() const
    {
        return registerValue(Register16::DEShadow);
    }

    inline UnsignedWord hlShadowRegisterValue() const
    {
        return registerValue(Register16::HLShadow);
    }

    /* 8-bit registers */
    inline UnsignedByte aRegisterValue() const
    {

        return registerValue(Register8::A);
    }

    inline UnsignedByte fRegisterValue() const
    {
        return registerValue(Register8::F);
    }

    inline UnsignedByte bRegisterValue() const
    {
        return registerValue(Register8::B);
    }

    inline UnsignedByte cRegisterValue() const
    {
        return registerValue(Register8::C);
    }

    inline UnsignedByte dRegisterValue() const
    {
        return registerValue(Register8::D);
    }

    inline UnsignedByte eRegisterValue() const
    {
        return registerValue(Register8::E);
    }

    inline UnsignedByte hRegisterValue() const
    {
        return registerValue(Register8::H);
    }

    inline UnsignedByte lRegisterValue() const
    {
        return registerValue(Register8::L);
    }

    inline UnsignedByte iRegisterValue() const
    {
        return registerValue(Register8::L);
    }

    inline UnsignedByte rRegisterValue() const
    {
        return registerValue(Register8::L);
    }

    inline UnsignedByte ixhRegisterValue() const
    {
        return registerValue(Register8::IXH);
    }

    inline UnsignedByte ixlRegisterValue() const
    {
        return registerValue(Register8::IXL);
    }

    inline UnsignedByte iyhRegisterValue() const
    {
        return registerValue(Register8::IYH);
    }

    inline UnsignedByte iylRegisterValue() const
    {
        return registerValue(Register8::IYL);
    }

    inline UnsignedByte aShadowRegisterValue() const
    {
        return registerValue(Register8::AShadow);
    }

    inline UnsignedByte fShadowRegisterValue() const
    {
        return registerValue(Register8::FShadow);
    }

    inline UnsignedByte bShadowRegisterValue() const
    {
        return registerValue(Register8::BShadow);
    }

    inline UnsignedByte cShadowRegisterValue() const
    {
        return registerValue(Register8::CShadow);
    }

    inline UnsignedByte dShadowRegisterValue() const
    {
        return registerValue(Register8::DShadow);
    }

    inline UnsignedByte eShadowRegisterValue() const
    {
        return registerValue(Register8::EShadow);
    }

    inline UnsignedByte hShadowRegisterValue() const
    {
        return registerValue(Register8::HShadow);
    }

    inline UnsignedByte lShadowRegisterValue() const
    {
        return registerValue(Register8::LShadow);
    }

    inline int interruptMode() const
    {
        return m_interruptMode;
    }

    inline bool iff1() const
    {
        return m_iff1;
    }

    inline bool iff2() const
    {
        return m_iff2;
    }

    /* Register setters */
    void setRegisterValue(Register16 reg, UnsignedWord value);

    void setRegisterValue(Register8 reg, UnsignedByte value);

    /* 16-bit Registers */
    void setAf(UnsignedWord value)
    {
        setRegisterValue(Register16::AF, value);
    }

    void setBc(UnsignedWord value)
    {
        setRegisterValue(Register16::BC, value);
    }

    void setDe(UnsignedWord value)
    {
        setRegisterValue(Register16::DE, value);
    }

    void setHl(UnsignedWord value)
    {
        setRegisterValue(Register16::HL, value);
    }

    void setSp(UnsignedWord value)
    {
        setRegisterValue(Register16::SP, value);
    }

    void setPc(UnsignedWord value)
    {
        setRegisterValue(Register16::PC, value);
    }

    void setIx(UnsignedWord value)
    {
        setRegisterValue(Register16::IX, value);
    }

    void setIy(UnsignedWord value)
    {
        setRegisterValue(Register16::IY, value);
    }

    void setAfShadow(UnsignedWord value)
    {
        setRegisterValue(Register16::AFShadow, value);
    }

    void setBcShadow(UnsignedWord value)
    {
        setRegisterValue(Register16::BCShadow, value);
    }

    void setDeShadow(UnsignedWord value)
    {
        setRegisterValue(Register16::DEShadow, value);
    }

    void setHlShadow(UnsignedWord value)
    {
        setRegisterValue(Register16::HLShadow, value);
    }

    /* 8-bit Registers */
    void setA(UnsignedByte value)
    {
        setRegisterValue(Register8::A, value);
    }

    void setF(UnsignedByte value)
    {
        setRegisterValue(Register8::F, value);
    }

    void setB(UnsignedByte value)
    {
        setRegisterValue(Register8::B, value);
    }

    void setC(UnsignedByte value)
    {
        setRegisterValue(Register8::C, value);
    }

    void setD(UnsignedByte value)
    {
        setRegisterValue(Register8::D, value);
    }

    void setE(UnsignedByte value)
    {
        setRegisterValue(Register8::E, value);
    }

    void setH(UnsignedByte value)
    {
        setRegisterValue(Register8::H, value);
    }

    void setL(UnsignedByte value)
    {
        setRegisterValue(Register8::L, value);
    }

    void setI(UnsignedByte value)
    {
        setRegisterValue(Register8::I, value);
    }

    void setR(UnsignedByte value)
    {
        setRegisterValue(Register8::R, value);
    }

    void setAShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::AShadow, value);
    }

    void setFShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::FShadow, value);
    }

    void setBShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::BShadow, value);
    }

    void setCShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::CShadow, value);
    }

    void setDShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::DShadow, value);
    }

    void setEShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::EShadow, value);
    }

    void setHShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::HShadow, value);
    }

    void setLShadow(UnsignedByte value)
    {
        setRegisterValue(Register8::LShadow, value);
    }

    inline void setIff1(bool iff1)
    {
        m_iff1 = iff1;
    }

    inline void setIff2(bool iff2)
    {
        m_iff2 = iff2;
    }

    inline int ramSize() const
    {
        return m_ramSize;
    }

    /* memory access methods - values provided in host byte order */
    inline Z80::UnsignedByte * memory() const
    {
        return m_ram;
    }

    inline bool cFlag() const
    {
        return (*m_f) & Z80_FLAG_C_MASK;
    }

    inline bool zFlag() const
    {
        return (*m_f) & Z80_FLAG_Z_MASK;
    }

    inline bool sFlag() const
    {
        return (*m_f) & Z80_FLAG_S_MASK;
    }

    inline bool pFlag() const
    {
        return (*m_f) & Z80_FLAG_P_MASK;
    }

    inline bool nFlag() const
    {
        return (*m_f) & Z80_FLAG_N_MASK;
    }

    inline bool hFlag() const
    {
        return (*m_f) & Z80_FLAG_H_MASK;
    }

    inline bool cShadowFlag() const
    {
        return (*m_fshadow) & Z80_FLAG_C_MASK;
    }

    inline bool zShadowFlag() const
    {
        return (*m_fshadow) & Z80_FLAG_Z_MASK;
    }

    inline bool sShadowFlag() const
    {
        return (*m_fshadow) & Z80_FLAG_S_MASK;
    }

    inline bool pShadowFlag() const
    {
        return (*m_fshadow) & Z80_FLAG_P_MASK;
    }

    inline bool nShadowFlag() const
    {
        return (*m_fshadow) & Z80_FLAG_N_MASK;
    }

    inline bool hShadowFlag() const
    {
        return (*m_fshadow) & Z80_FLAG_H_MASK;
    }

    inline bool carryFlag() const
    {
        return cFlag();
    }

    inline bool zeroFlag() const
    {
        return zFlag();
    }

    inline bool signFlag() const
    {
        return sFlag();
    }

    inline bool parityFlag() const
    {
        return pFlag();
    }

    inline bool vFlag() const
    {
        return pFlag();
    }

    inline bool overflowFlag() const
    {
        return pFlag();
    }

    inline bool pvFlag() const
    {
        return pFlag();
    }

    inline bool negationFlag() const
    {
        return nFlag();
    }

    inline bool halfcarryFlag() const
    {
        return hFlag();
    }

    inline bool carryShadowFlag() const
    {
        return cShadowFlag();
    }

    inline bool zeroShadowFlag() const
    {
        return zShadowFlag();
    }

    inline bool signShadowFlag() const
    {
        return sShadowFlag();
    }

    inline bool parityShadowFlag() const
    {
        return pShadowFlag();
    }

    inline bool vShadowFlag() const
    {
        return pShadowFlag();
    }

    inline bool overflowShadowFlag() const
    {
        return pShadowFlag();
    }

    inline bool pvShadowFlag() const
    {
        return pShadowFlag();
    }

    inline bool negationShadowFlag() const
    {
        return nShadowFlag();
    }

    inline bool halfcarryShadowFlag() const
    {
        return hShadowFlag();
    }

    inline UnsignedByte peekUnsigned(int addr) const
    {
        if (addr < 0 || addr >= m_ramSize) {
            return 0;
        }

        return m_ram[addr];
    }

    UnsignedWord peekUnsignedWord(int addr) const;

    inline SignedByte peekSigned(int addr) const
    {
        return SignedByte(peekUnsigned(addr));
    }

    inline SignedWord peekSignedWord(int addr) const
    {
        return SignedByte(peekUnsignedWord(addr));
    }

    inline void pokeUnsigned(int addr, UnsignedByte value)
    {
        if (addr < 0 || addr > m_ramSize) {
            return;
        }

        m_ram[addr] = value;
    }

    inline void pokeSigned(int addr, SignedByte value)
    {
        pokeUnsigned(addr, UnsignedByte(value));
    }

    // v is in host byte order
    void pokeUnsignedWord(int addr, UnsignedWord value);

    void pokeSignedWord(int addr, SignedWord value);

    void reset();

    void start();

    bool connectIODevice(Z80::UnsignedWord port, Z80IODevice * device);

    void disconnectIODevice(Z80::UnsignedWord port, Z80IODevice * device);

    /* call this to put an instruction in the Z80 CPU before calling
     * interrupt() if the IO device is using Mode 0 interrupts. the
     * instruction is copied so it can be deallocated if required immediately
     * after the call */
    void setInterruptMode0Instruction(Z80::UnsignedByte * instructions, int bytes);

    void interrupt();

    void nmi();

    bool load(unsigned char * data, int addr, int length);

    bool execute(const UnsignedByte * instruction, bool doPc = true, int * cycles = 0, int * size = 0);

    /* fetches and executes a single instruction, returns the cycle cost */
    int fetchExecuteCycle();

protected:
    /* doPc is altered to be false if the instruction is a jump that is taken;
     * cycles is filled with clock cycles consumed; size is filled with byte
     * size of instruction and operands; doPc is set to false if the
     * instruction has modified the PC. returns true if execution of
     * instruction was successful, false otherwise */
    bool executePlainInstruction(const UnsignedByte * instruction, bool * doPc = nullptr, int * cycles = nullptr,
                                 int * size = nullptr);

    bool executeCbInstruction(const UnsignedByte * instruction, int * cycles = nullptr, int * size = nullptr);

    bool executeEdInstruction(const UnsignedByte * instruction, bool * doPc = nullptr, int * cycles = nullptr,
                              int * size = nullptr);

    bool executeDdOrFdInstruction(UnsignedWord & reg, const UnsignedByte * instruction, bool * doPc = nullptr,
                                  int * cycles = nullptr, int * size = nullptr);

    bool executeDdcbOrFdcbInstruction(UnsignedWord & reg, const UnsignedByte * instruction, int * cycles = nullptr,
                                      int * size = nullptr);
//		bool executeFdInstruction( const UnsignedByte * instruction, bool * doPc = 0, int * cycles = 0, int * size = 0 );


private:
    struct IODeviceConnection
    {
        Z80IODevice * device;
        int port;
    };

    /* NEVER call this method more than once. most subclasses will never need
     * to call it as the Z80 constructor makes sure the call is made */
    void init();

    /* Registers */
    UnsignedWord m_af;
    UnsignedWord m_bc;
    UnsignedWord m_de;
    UnsignedWord m_hl;
    UnsignedWord m_sp;
    UnsignedWord m_pc;
    UnsignedWord m_ix;
    UnsignedWord m_iy;
    UnsignedByte m_i;
    UnsignedByte m_r;
    UnsignedWord m_afshadow;
    UnsignedWord m_bcshadow;
    UnsignedWord m_deshadow;
    UnsignedWord m_hlshadow;

    /* 8-bit components of 16-bit registers - set up to point to the correct byte in the 16-bit register in the constructor */
    UnsignedByte * m_a;
    UnsignedByte * m_f;
    UnsignedByte * m_b;
    UnsignedByte * m_c;
    UnsignedByte * m_d;
    UnsignedByte * m_e;
    UnsignedByte * m_h;
    UnsignedByte * m_l;
    UnsignedByte * m_ixh;
    UnsignedByte * m_ixl;
    UnsignedByte * m_iyh;
    UnsignedByte * m_iyl;
    UnsignedByte * m_ashadow;
    UnsignedByte * m_fshadow;
    UnsignedByte * m_bshadow;
    UnsignedByte * m_cshadow;
    UnsignedByte * m_dshadow;
    UnsignedByte * m_eshadow;
    UnsignedByte * m_hshadow;
    UnsignedByte * m_lshadow;

    // memory
    Z80::UnsignedByte * m_ram;
    int m_ramSize;
    bool m_nmiPending;
    bool m_interruptRequested;
    bool m_iff1;
    bool m_iff2;
    char m_interruptMode;
    unsigned int m_plain_opcode_cycles[256];

    // 16-bit opcodes starting 0xdd and 0xfd use the IX and IY registers respectively for identical instructions, so
    // their opcode costs are identical
    // TODO all 0xddcb or 0xfdcb opcodes are 4 bytes and consume 23 cycles except BIT instructions which are 20
    unsigned int m_ddorfd_opcode_cycles[256];
    unsigned int m_fd_opcode_cycles[256];
    unsigned int m_ed_opcode_cycles[256];
    unsigned int m_cb_opcode_cycles[256];
    unsigned int m_plain_opcode_size[256];
    static const unsigned int DdOrFdOpcodeSize[256];
    unsigned int m_fd_opcode_size[256];
    unsigned int m_ed_opcode_size[256];
    unsigned long long m_clockSpeed;    /* clock speed in Hz */
    IODeviceConnection ** m_ioDeviceConnections;
    int m_ioDeviceCount;
    int m_ioDeviceCapacity;
    Z80::UnsignedByte m_interruptMode0Instruction[16];    /* cache for instructions placed by IO devices when IM is 0 */
};

#endif // Z80_H
