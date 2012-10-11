#ifndef COMPUTER_H
#define COMPUTER_H

#include "array.h"
class Cpu;

class Computer {
	public:
		Computer( int memsize = 0 );
		Computer( unsigned char * mem, int memsize );
		virtual ~Computer( void );

		Cpu * cpu( int i = 0 );

		inline Array<Cpu *> cpus( void ) const {
			return m_cpus;
		}

		inline unsigned char * memory( void ) const {
			return m_ram;
		}

		inline int memorySize( void ) const {
			return m_ramSize;
		}

		inline bool addCpu( Cpu * cpu ) {
			return m_cpus.append(cpu);
		}

		inline void removeCpu( Cpu * cpu ) {
			m_cpus.removeAll(cpu);
		}

		virtual void reset( void ) = 0;
		virtual void run( int instructionCount ) = 0;

	protected:
		Array<Cpu *> m_cpus;
		unsigned char * m_ram;
		int m_ramSize;

	private:
		bool m_myRam;
};

#endif // COMPUTER_H
