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

			[[nodiscard]] std::uint8_t * displayMemory() const override
			{
				return memory() + DisplayMemoryOffset;
			}

			[[nodiscard]] int displayMemorySize() const override
			{
				return DisplayMemorySize;
			}
    };
}

#endif // SPECTRUM_SPECTRUM48_H
