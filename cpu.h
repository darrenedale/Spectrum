#ifndef CPU_H
#define CPU_H

class Cpu {
	public:
		Cpu( unsigned char * mem, int memSize );
		virtual ~Cpu( void );

		void setMemory( unsigned char * mem, int memsize );

		inline unsigned char * memory( void ) {
			return m_ram;
		}

		inline int memorySize( void ) {
			return m_ramSize;
		}

		inline void setClockSpeed( unsigned long long hz ) {
			m_clockSpeed = hz;
		}

		inline void setClockSpeedMHz( double mhz ) {
			m_clockSpeed = (unsigned long long)(mhz * 1000000);
		}

		inline unsigned long long clockSpeed( void ) {
			return m_clockSpeed;
		}

		inline double clockSpeedMHz( void ) {
			return double(m_clockSpeed) / 1000000.0;
		}

	protected:
		unsigned char * m_ram;
		int m_ramSize;
		int m_clockSpeed;
};

#endif // CPU_H
