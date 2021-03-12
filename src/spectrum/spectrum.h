#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>

#include "../computer.h"
#include "z80.h"

namespace Spectrum
{
	class DisplayDevice;
	class Keyboard;

	class Spectrum
    :	public Computer<>
    {
		public:
            static constexpr const int DisplayMemoryOffset = 0x4000;
            static constexpr const int DisplayMemorySize = 6912;
            static constexpr const int DefaultClockSpeed = 3500000;

			explicit Spectrum(int = 65536, uint8_t * mem = nullptr);
			~Spectrum() override;

			inline Z80 * z80() const
            {
			    return dynamic_cast<Z80 *>(cpu());
            }

			inline int nmiCounter() const
			{
				return m_interruptCycleCounter;
			}

			std::uint8_t * displayMemory() const
			{
				return memory() + DisplayMemoryOffset;
			}

			int displayMemorySize() const
			{
				return DisplayMemorySize;
			}

			void reset() override;
			void run(int instructionCount) override;

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

			void addDisplayDevice(DisplayDevice * dev);

			void removeDisplayDevice(DisplayDevice * dev)
			{
			    const auto pos = std::find(m_displayDevices.begin(), m_displayDevices.end(), dev);

			    if (pos == m_displayDevices.end()) {
			        return;
			    }

				m_displayDevices.erase(pos, pos);
			}

			void setKeyboard(Keyboard * keyboard);

            [[nodiscard]] Keyboard * keyboard() const
            {
                return m_keyboard;
            }

			inline void refreshDisplays() const;

		protected:
	        void clearMemory() const;
			bool loadRom(const std::string & fileName) const;

		private:
			int m_interruptCycleCounter;
			bool m_constrainExecutionSpeed;
			std::vector<DisplayDevice *> m_displayDevices;
            Keyboard * m_keyboard;
    };
}

#endif // SPECTRUM_H
