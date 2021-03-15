#ifndef COMPUTER_H
#define COMPUTER_H

#include <algorithm>
#include <cstdint>
#include <vector>
#include <cassert>

class Cpu;

template<class byte_t = std::uint8_t>
class Computer {
	public:
		explicit Computer(int memSize = 0, byte_t * mem = nullptr )
		:	m_ram(mem),
            m_ramSize(memSize),
            m_myRam(!mem)
        {
		    assert(0 <= memSize);

		    if (!m_ram) {
                m_ram = new byte_t[memSize];
            }
        }

		virtual ~Computer()
        {
            if (m_myRam) {
                delete[] m_ram;
            }

            m_ram = nullptr;
            m_ramSize = 0;
        }

		Cpu * cpu( int idx = 0 ) const
        {
		    assert(0 <= idx && m_cpus.size() > idx);
		    return m_cpus[idx];
        }

		inline const std::vector<Cpu *> & cpus() const
		{
			return m_cpus;
		}

		inline std::vector<Cpu *> & cpus()
		{
			return m_cpus;
		}

		inline byte_t * memory() const
		{
			return m_ram;
		}

		inline int memorySize() const
		{
			return m_ramSize;
		}

		inline void addCpu(Cpu * cpu)
		{
		    assert(cpu);
			m_cpus.push_back(cpu);
		}

		inline void removeCpu(Cpu * cpu)
		{
		    assert(cpu);
		    const auto pos = std::find(m_cpus.begin(), m_cpus.end(), cpu);

		    if (pos == m_cpus.end()) {
		        return;
		    }

			m_cpus.erase(pos, pos);
		}

		/**
		 * Provide the computer with new memory that it borrows.
		 *
		 * @param memory
		 * @param size
		 */
		inline void setMemory(byte_t * memory, int size)
        {
		    if (m_myRam) {
		        delete[] m_ram;
		        m_myRam = false;
		    }

		    // TODO pass the memory to the CPUs
		    m_ram = memory;
		    m_ramSize = size;
        }

        /**
         * Proivide the computer with new memory that it owns.
         *
         * @param memory
         * @param size
         */
		inline void giveMemory(byte_t * memory, int size)
        {
		    if (m_myRam) {
		        delete[] m_ram;
		    }

            // TODO pass the memory to the CPUs
		    m_myRam = true;
		    m_ram = memory;
		    m_ramSize = size;
        }

		virtual void reset() = 0;
		virtual void run(int instructionCount) = 0;

	protected:
		std::vector<Cpu *> m_cpus;
		std::uint8_t * m_ram;
		int m_ramSize;

	private:
		bool m_myRam;
};

#endif // COMPUTER_H
