#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>

#include "../computer.h"
#include "../z80/z80.h"

#define SPECTRUM_OFFSET_DISPLAYFILE 0x4000
#define SPECTRUM_DISPLAYFILE_SIZE 6912

namespace Spectrum {
	class SpectrumDisplayDevice;

	class Spectrum
    :	public Computer<>
    {
		public:
            static constexpr const int DefaultClockSpeed = 3500000;

			explicit Spectrum(int = 65536, uint8_t * mem = nullptr);
			~Spectrum() override;

			inline Z80::Z80 * z80() const
            {
			    return dynamic_cast<Z80::Z80 *>(cpu());
            }

			inline int nmiCounter() const
			{
				return m_nmiCycleCounter;
			}

			std::uint8_t * displayMemory() const
			{
				return memory() + SPECTRUM_OFFSET_DISPLAYFILE;
			}

			int displayMemorySize() const
			{
				return SPECTRUM_DISPLAYFILE_SIZE;
			}

			virtual void reset();
			virtual void run(int instructionCount);

			/**
			 * If set, the Spectrum will be constrained to run no faster than the clock speed set.
			 *
			 * If not set, it will run as fast as the host CPU will allow.
			 *
			 * @param constraint
			 */
			void setExecutionSpeedConstrained(bool constrain)
            {
			    m_constrainExecutionSpeed = constrain;
            }

            /**
             * Check whether the Spectrum is running at approximately the speed determined by the clock speed.
             *
             * @return
             */
            bool executionSpeedConstrained() const
            {
			    return m_constrainExecutionSpeed;
            }

			void addDisplayDevice( SpectrumDisplayDevice * dev )
			{
				m_displayDevices.push_back(dev);
			}

			void removeDisplayDevice(SpectrumDisplayDevice * dev)
			{
			    const auto pos = std::find(m_displayDevices.begin(), m_displayDevices.end(), dev);

			    if (pos == m_displayDevices.end()) {
			        return;
			    }

				m_displayDevices.erase(pos, pos);
			}

		protected:
	        void clearMemory() const;
			bool loadRom(const std::string & fileName) const;
			inline void refreshDisplays() const;

		private:
			int m_nmiCycleCounter;
			bool m_constrainExecutionSpeed;
			std::vector<SpectrumDisplayDevice *> m_displayDevices;
    };
}

#endif // SPECTRUM_H
