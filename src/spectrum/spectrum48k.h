#ifndef SPECTRUM_SPECTRUM48_H
#define SPECTRUM_SPECTRUM48_H

#include <string>

#include "basespectrum.h"

namespace Spectrum
{
	class Spectrum48k
    : public BaseSpectrum
    {
		public:
            static constexpr const int DisplayMemoryOffset = 0x4000;
            static constexpr const int DisplayMemorySize = 6912;

			Spectrum48k();
			explicit Spectrum48k(const std::string & romFile);
			~Spectrum48k() override;

			/**
			 * It's safe to use the returned pointer as a pointer to a contiguous block of 16kb of Spectrum RAM because
			 * this page is never paged out from the address 0x4000.
			 */
			[[nodiscard]] std::uint8_t * displayMemory() const override
			{
				return memory()->pointerTo(DisplayMemoryOffset);
			}

			[[nodiscard]] int displayMemorySize() const override
			{
				return DisplayMemorySize;
			}
    };
}

#endif // SPECTRUM_SPECTRUM48_H
