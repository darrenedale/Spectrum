#ifndef Z80_H
#define Z80_H

#include "types.h"
#include "cpu.h"

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
:	public Cpu {
	public:
		typedef unsigned char UnsignedByte;
		typedef unsigned short UnsignedWord;
		typedef signed char SignedByte;
		typedef signed short SignedWord;

		enum Register16 {
			RegAF,
			RegBC,
			RegDE,
			RegHL,
			RegIX,
			RegIY,
			RegSP,
			RegPC,
			RegAFShadow,
			RegBCShadow,
			RegDEShadow,
			RegHLShadow
		};

		enum Register8 {
			RegA,
			RegF,
			RegB,
			RegC,
			RegD,
			RegE,
			RegH,
			RegL,
			RegIXH,
			RegIXL,
			RegIYH,
			RegIYL,
			RegI,
			RegR,
			RegAShadow,
			RegFShadow,
			RegBShadow,
			RegCShadow,
			RegDShadow,
			RegEShadow,
			RegHShadow,
			RegLShadow
		};

		enum ByteOrder {
			BigEndian,
			LittleEndian
		};

		Z80( unsigned char * mem, int memsize );
		virtual ~Z80( void );

		static inline ByteOrder z80ByteOrder( void ) {
			return LittleEndian;
		}

		static inline ByteOrder hostByteOrder( void ) {
			static int i = 1;
			return (0 == (unsigned char *) &i ? LittleEndian : BigEndian);
		}

		static inline short swapByteOrder( short v ) {
			return (((v & 0xff00) >> 8) & 0x00ff) | (((v & 0x00ff) << 8) & 0xff00);
		}

		static inline short z80ToHostByteOrder( short v ) {
			return (z80ByteOrder() == hostByteOrder() ? v : swapByteOrder(v));
		}

		static inline short hostToZ80ByteOrder( short v ) {
			if(z80ByteOrder() == hostByteOrder()) return v;
			return swapByteOrder(v);
		}

		static inline bool isEvenParity( UnsignedByte v );
		static inline bool isEvenParity( UnsignedWord v );

		bool isValid( void ) const {
			return m_ram;
		}

		/* register access methods - values provided in host byte order */
		/* register getters */
		UnsignedWord registerValue( Register16 reg ) const;
		UnsignedByte registerValue( Register8 reg ) const;

		/* 16-bit registers */
		inline UnsignedWord afRegisterValue( void ) const {
			return registerValue(RegAF);
		}

		inline UnsignedWord bcRegisterValue( void ) const {
			return registerValue(RegBC);
		}

		inline UnsignedWord deRegisterValue( void ) const {
			return registerValue(RegDE);
		}

		inline UnsignedWord hlRegisterValue( void ) const {
			return registerValue(RegHL);
		}

		inline UnsignedWord ixRegisterValue( void ) const {
			return registerValue(RegIX);
		}

		inline UnsignedWord iyRegisterValue( void ) const {
			return registerValue(RegIY);
		}

		/* stack pointer */
		inline UnsignedWord sp( void ) const {
			return registerValue(RegSP);
		}

		inline UnsignedWord stackPointer( void ) const {
			return sp();
		}

		/* program counter */
		inline UnsignedWord pc( void ) const {
			return registerValue(RegPC);
		}

		inline UnsignedWord programCounter( void ) const {
			return pc();
		}

		inline UnsignedWord afShadowRegisterValue( void ) const {
			return registerValue(RegAFShadow);
		}

		inline UnsignedWord bcShadowRegisterValue( void ) const {
			return registerValue(RegBCShadow);
		}

		inline UnsignedWord deShadowRegisterValue( void ) const {
			return registerValue(RegDEShadow);
		}

		inline UnsignedWord hlShadowRegisterValue( void ) const {
			return registerValue(RegHLShadow);
		}

		/* 8-bit registers */
		inline UnsignedByte aRegisterValue( void ) const {
			return registerValue(RegA);
		}

		inline UnsignedByte fRegisterValue( void ) const {
			return registerValue(RegF);
		}

		inline UnsignedByte bRegisterValue( void ) const {
			return registerValue(RegB);
		}

		inline UnsignedByte cRegisterValue( void ) const {
			return registerValue(RegC);
		}

		inline UnsignedByte dRegisterValue( void ) const {
			return registerValue(RegD);
		}

		inline UnsignedByte eRegisterValue( void ) const {
			return registerValue(RegE);
		}

		inline UnsignedByte hRegisterValue( void ) const {
			return registerValue(RegH);
		}

		inline UnsignedByte lRegisterValue( void ) const {
			return registerValue(RegL);
		}

		inline UnsignedByte iRegisterValue( void ) const {
			return registerValue(RegL);
		}

		inline UnsignedByte rRegisterValue( void ) const {
			return registerValue(RegL);
		}

		inline UnsignedByte ixhRegisterValue( void ) const {
			return registerValue(RegIXH);
		}

		inline UnsignedByte ixlRegisterValue( void ) const {
			return registerValue(RegIXL);
		}

		inline UnsignedByte iyhRegisterValue( void ) const {
			return registerValue(RegIYH);
		}

		inline UnsignedByte iylRegisterValue( void ) const {
			return registerValue(RegIYL);
		}

		inline UnsignedByte aShadowRegisterValue( void ) const {
			return registerValue(RegAShadow);
		}

		inline UnsignedByte fShadowRegisterValue( void ) const {
			return registerValue(RegFShadow);
		}

		inline UnsignedByte bShadowRegisterValue( void ) const {
			return registerValue(RegBShadow);
		}

		inline UnsignedByte cShadowRegisterValue( void ) const {
			return registerValue(RegCShadow);
		}

		inline UnsignedByte dShadowRegisterValue( void ) const {
			return registerValue(RegDShadow);
		}

		inline UnsignedByte eShadowRegisterValue( void ) const {
			return registerValue(RegEShadow);
		}

		inline UnsignedByte hShadowRegisterValue( void ) const {
			return registerValue(RegHShadow);
		}

		inline UnsignedByte lShadowRegisterValue( void ) const {
			return registerValue(RegLShadow);
		}

		inline int interruptMode( void ) const {
			return m_interruptMode;
		}

		inline bool iff1( void ) const {
			return m_iff1;
		}

		inline bool iff2( void ) const {
			return m_iff2;
		}

		/* register setters */
		void setRegisterValue( Register16 reg, UnsignedWord value);
		void setRegisterValue( Register8 reg, UnsignedByte value);

		/* 16-bit registers */
		void setAf( UnsignedWord value ) {
			setRegisterValue(RegAF, value);
		}

		void setBc( UnsignedWord value ) {
			setRegisterValue(RegBC, value);
		}

		void setDe( UnsignedWord value ) {
			setRegisterValue(RegDE, value);
		}

		void setHl( UnsignedWord value ) {
			setRegisterValue(RegHL, value);
		}

		void setSp( UnsignedWord value ) {
			setRegisterValue(RegSP, value);
		}

		void setPc( UnsignedWord value ) {
			setRegisterValue(RegPC, value);
		}

		void setIx( UnsignedWord value ) {
			setRegisterValue(RegIX, value);
		}

		void setIy( UnsignedWord value ) {
			setRegisterValue(RegIY, value);
		}

		void setAfShadow( UnsignedWord value ) {
			setRegisterValue(RegAFShadow, value);
		}

		void setBcShadow( UnsignedWord value ) {
			setRegisterValue(RegBCShadow, value);
		}

		void setDeShadow( UnsignedWord value ) {
			setRegisterValue(RegDEShadow, value);
		}

		void setHlShadow( UnsignedWord value ) {
			setRegisterValue(RegHLShadow, value);
		}

		/* 8-bit registers*/
		void setA( UnsignedByte value ) {
			setRegisterValue(RegA, value);
		}

		void setF( UnsignedByte value ) {
			setRegisterValue(RegF, value);
		}

		void setB( UnsignedByte value ) {
			setRegisterValue(RegB, value);
		}

		void setC( UnsignedByte value ) {
			setRegisterValue(RegC, value);
		}

		void setD( UnsignedByte value ) {
			setRegisterValue(RegD, value);
		}

		void setE( UnsignedByte value ) {
			setRegisterValue(RegE, value);
		}

		void setH( UnsignedByte value ) {
			setRegisterValue(RegH, value);
		}

		void setL( UnsignedByte value ) {
			setRegisterValue(RegL, value);
		}

		void setI( UnsignedByte value ) {
			setRegisterValue(RegI, value);
		}

		void setR( UnsignedByte value ) {
			setRegisterValue(RegR, value);
		}

		void setAShadow( UnsignedByte value ) {
			setRegisterValue(RegAShadow, value);
		}

		void setFShadow( UnsignedByte value ) {
			setRegisterValue(RegFShadow, value);
		}

		void setBShadow( UnsignedByte value ) {
			setRegisterValue(RegBShadow, value);
		}

		void setCShadow( UnsignedByte value ) {
			setRegisterValue(RegCShadow, value);
		}

		void setDShadow( UnsignedByte value ) {
			setRegisterValue(RegDShadow, value);
		}

		void setEShadow( UnsignedByte value ) {
			setRegisterValue(RegEShadow, value);
		}

		void setHShadow( UnsignedByte value ) {
			setRegisterValue(RegHShadow, value);
		}

		void setLShadow( UnsignedByte value ) {
			setRegisterValue(RegLShadow, value);
		}

		inline int ramSize( void ) const {
			return m_ramSize;
		}

		/* memory access methods - values provided in host byte order */
		inline const Z80::UnsignedByte * memory( void ) const {
			return m_ram;
		}

		inline bool cFlag( void ) const { return (*m_f) & Z80_FLAG_C_MASK; }
		inline bool zFlag( void ) const { return (*m_f) & Z80_FLAG_Z_MASK; }
		inline bool sFlag( void ) const { return (*m_f) & Z80_FLAG_S_MASK; }
		inline bool pFlag( void ) const { return (*m_f) & Z80_FLAG_P_MASK; }
		inline bool nFlag( void ) const { return (*m_f) & Z80_FLAG_N_MASK; }
		inline bool hFlag( void ) const { return (*m_f) & Z80_FLAG_H_MASK; }

		inline bool cShadowFlag( void ) const { return (*m_fshadow) & Z80_FLAG_C_MASK; }
		inline bool zShadowFlag( void ) const { return (*m_fshadow) & Z80_FLAG_Z_MASK; }
		inline bool sShadowFlag( void ) const { return (*m_fshadow) & Z80_FLAG_S_MASK; }
		inline bool pShadowFlag( void ) const { return (*m_fshadow) & Z80_FLAG_P_MASK; }
		inline bool nShadowFlag( void ) const { return (*m_fshadow) & Z80_FLAG_N_MASK; }
		inline bool hShadowFlag( void ) const { return (*m_fshadow) & Z80_FLAG_H_MASK; }

		inline bool carryFlag( void ) const { return cFlag(); }
		inline bool zeroFlag( void ) const { return zFlag(); }
		inline bool signFlag( void ) const { return sFlag(); }
		inline bool parityFlag( void ) const { return pFlag(); }
		inline bool vFlag( void ) const { return pFlag(); }
		inline bool overflowFlag( void ) const { return pFlag(); }
		inline bool pvFlag( void ) const { return pFlag(); }
		inline bool negationFlag( void ) const { return nFlag(); }
		inline bool halfcarryFlag( void ) const { return hFlag(); }

		inline bool carryShadowFlag( void ) const { return cShadowFlag(); }
		inline bool zeroShadowFlag( void ) const { return zShadowFlag(); }
		inline bool signShadowFlag( void ) const { return sShadowFlag(); }
		inline bool parityShadowFlag( void ) const { return pShadowFlag(); }
		inline bool vShadowFlag( void ) const { return pShadowFlag(); }
		inline bool overflowShadowFlag( void ) const { return pShadowFlag(); }
		inline bool pvShadowFlag( void ) const { return pShadowFlag(); }
		inline bool negationShadowFlag( void ) const { return nShadowFlag(); }
		inline bool halfcarryShadowFlag( void ) const { return hShadowFlag(); }

		inline UnsignedByte peekUnsigned( int addr ) const {
			if(addr < 0 || addr >= m_ramSize) return 0;
			return m_ram[addr];
		}

		UnsignedWord peekUnsignedWord( int addr ) const;

		inline SignedByte peekSigned( int addr ) const {
			return SignedByte(peekUnsigned(addr));
		}

		inline SignedWord peekSignedWord( int addr ) const {
			return SignedByte(peekUnsignedWord(addr));
		}

		inline void pokeUnsigned( int addr, UnsignedByte v ) {
			if(addr < 0 || addr > m_ramSize) return;
			m_ram[addr] = v;
		}

		inline void pokeSigned( int addr, SignedByte v ) {
			pokeUnsigned(addr, UnsignedByte(v));
		}

		/* v is in host byte order */
		void pokeUnsignedWord( int addr, UnsignedWord v );
		void pokeSignedWord( int addr, SignedWord v );

		void reset( void );
		void start( void );

		bool connectIODevice( Z80::UnsignedWord port, Z80IODevice * device );
		void disconnectIODevice( Z80::UnsignedWord port, Z80IODevice * device );

		/* call this to put an instruction in the Z80 CPU before calling
		 * interrupt() if the IO device is using Mode 0 interrupts. the
		 * instruction is copied so it can be deallocated if required immediately
		 * after the call */
		void setInterruptMode0Instruction( Z80::UnsignedByte * instructions, int bytes );
		void interrupt( void );
		void nmi( void );

		bool load( unsigned char * data, int addr, int length );
		bool execute( const UnsignedByte * instruction, bool doPc = true, int * cycles = 0, int * size = 0 );

		/* fetches and executes a single instruction, returns the cycle cost */
		int fetchExecuteCycle( void );

	protected:
		/* doPc is altered to be false if the instruction is a jump that is taken;
		 * cycles is filled with clock cycles consumed; size is filled with byte
		 * size of instruction and operands; doPc is set to false if the
		 * instruction has modified the PC. returns true if execution of
		 * instruction was successful, false otherwise */
		bool executePlainInstruction( const UnsignedByte * instruction, bool * doPc = 0, int * cycles = 0, int * size = 0 );
		bool executeCbInstruction( const UnsignedByte * instruction, int * cycles = 0, int * size = 0 );
		bool executeEdInstruction( const UnsignedByte * instruction, bool * doPc = 0, int * cycles = 0, int * size = 0 );
		bool executeDdOrFdInstruction( UnsignedWord & reg, const UnsignedByte * instruction, bool * doPc = 0, int * cycles = 0, int * size = 0 );
		bool executeDdcbOrFdcbInstruction( UnsignedWord & reg, const UnsignedByte * instruction, int * cycles = 0, int * size = 0 );
//		bool executeFdInstruction( const UnsignedByte * instruction, bool * doPc = 0, int * cycles = 0, int * size = 0 );


	private:
		struct IODeviceConnection {
			Z80IODevice * device;
			int port;
		};

		/* NEVER call this method more than once. most subclasses will never need
		 * to call it as the Z80 constructor makes sure the call is made */
		void init( void );

		/* registers */
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

		/* memory */
		Z80::UnsignedByte * m_ram;
		int m_ramSize;
		bool m_nmiPending, m_interruptRequested, m_iff1, m_iff2;
		char m_interruptMode;
		unsigned int m_plain_opcode_cycles[256];

		/* 16-bit opcodes starting 0xdd and oxfd use the IX and IY registers
		 * respectively for identical instructions, so their opcode costs are
		 * identical */
		/* TODO all 0xddcb or 0xfdcb opcodes are 4 bytes and consume 23 cycles except
		 * except BIT instructions which are 20 cycles */
		unsigned int m_ddorfd_opcode_cycles[256];
		unsigned int m_fd_opcode_cycles[256];
		unsigned int m_ed_opcode_cycles[256];
		unsigned int m_cb_opcode_cycles[256];
		unsigned int m_plain_opcode_size[256];
		static const unsigned int DdOrFdOpcodeSize[256];
		unsigned int m_fd_opcode_size[256];
		unsigned int m_ed_opcode_size[256];
		unsigned long long m_clockSpeed;	/* clock speed in Hz */
		IODeviceConnection ** m_ioDeviceConnections;
		int m_ioDeviceCount, m_ioDeviceCapacity;
		Z80::UnsignedByte m_interrruptMode0Instruction[16];	/* cache for instructions placed by IO devices when IM is 0 */
};

#endif // Z80_H
