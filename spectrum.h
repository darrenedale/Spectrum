#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "computer.h"
#include "array.h"

#define SPECTRUM_OFFSET_DISPLAYFILE 0x4000
#define SPECTRUM_DISPLAYFILE_SIZE 6912

namespace Spectrum {
	class SpectrumDisplayDevice;

	class Spectrum
	:	public Computer {
		public:
			Spectrum( int memsize = 65536 );
			Spectrum( unsigned char * mem, int memsize );
			virtual ~Spectrum( void );

			void init( void );

			inline int nmiCounter( void ) const {
				return m_nmiCycleCounter;
			}

			unsigned char * displayMemory( void ) const {
				return memory() + SPECTRUM_OFFSET_DISPLAYFILE;
			}

			int displayMemorySize( void ) const {
				return SPECTRUM_DISPLAYFILE_SIZE;
			}

			virtual void reset( void );
			virtual void run( int instructionCount );

			void addDisplayDevice( SpectrumDisplayDevice * dev ) {
				if(dev) m_displayDevices.append(dev);
			}

			void removeDisplayDevice( SpectrumDisplayDevice * dev ) {
				if(dev) m_displayDevices.removeAll(dev);
			}

		protected:
			inline void refreshDisplays( void );

		private:
			int m_nmiCycleCounter;
			Array<SpectrumDisplayDevice *> m_displayDevices;
	};
}

#endif // SPECTRUM_H
