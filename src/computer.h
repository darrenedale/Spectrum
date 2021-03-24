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
		: m_memory(mem),
          m_memorySize(memSize),
          m_memoryOwned(!mem)
        {
		    assert(0 <= memSize);

		    if (!m_memory) {
                m_memory = new byte_t[memSize];
            }
        }

		virtual ~Computer()
        {
            if (m_memoryOwned) {
                delete[] m_memory;
            }

            m_memory = nullptr;
            m_memorySize = 0;
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
			return m_memory;
		}

		inline int memorySize() const
		{
			return m_memorySize;
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
		    if (m_memoryOwned) {
		        delete[] m_memory;
                m_memoryOwned = false;
		    }

		    // TODO pass the memory to the CPUs
		    m_memory = memory;
            m_memorySize = size;
        }

        /**
         * Proivide the computer with new memory that it owns.
         *
         * @param memory
         * @param size
         */
		inline void giveMemory(byte_t * memory, int size)
        {
		    if (m_memoryOwned) {
		        delete[] m_memory;
		    }

            // TODO pass the memory to the CPUs
		    m_memoryOwned = true;
            m_memory = memory;
            m_memorySize = size;
        }

		virtual void reset() = 0;
		virtual void run(int instructionCount) = 0;

	protected:
		std::vector<Cpu *> m_cpus;
		std::uint8_t * m_memory;
		int m_memorySize;

	private:
		bool m_memoryOwned;
};

#endif // COMPUTER_H
