#ifndef SPECTRUMDISPLAYDEVICE_H
#define SPECTRUMDISPLAYDEVICE_H

#include <cstdint>

namespace Spectrum
{
    /**
     * Interface for display devices for Spectrums.
     */
	class SpectrumDisplayDevice
	{
		public:
			virtual void redrawDisplay( const uint8_t * displayMemory ) = 0;
	};
}

#endif // SPECTRUMDISPLAYDEVICE_H
