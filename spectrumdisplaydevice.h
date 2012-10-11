#ifndef SPECTRUMDISPLAYDEVICE_H
#define SPECTRUMDISPLAYDEVICE_H

namespace Spectrum {
	class SpectrumDisplayDevice {
		public:
			SpectrumDisplayDevice( void );
			virtual ~SpectrumDisplayDevice( void );

			virtual void redrawDisplay( const unsigned char * displayMemory ) = 0;
	};
}

#endif // SPECTRUMDISPLAYDEVICE_H
