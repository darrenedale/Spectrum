#ifndef COMPUTER_H
#define COMPUTER_H

#include <algorithm>
#include <cstdint>
#include <vector>
#include <cassert>

#include "memory.h"

class Cpu;

template<class byte_t = std::uint8_t>
class Computer
{
public:
    using ByteType = byte_t;
    using MemoryType = Memory<ByteType>;

    /**
     * Initialise a new Computer with memory of a given size, which it owns.
     *
     * The computer will create its own linear, fully-allocated memory, which it will own and destroy when it is
     * destroyed.
     *
     * @param memSize
     */
    explicit Computer(typename MemoryType::Size memSize = 0)
    : m_memory(new MemoryType(memSize)),
      m_memoryOwned(true)
    {
        assert(0 <= memSize);
    }

    /**
     * Initialise a new Computer with pre-existing memory.
     *
     * The computer will only borrow the provided memory, it is up to the caller to ensure that the memory is destroyed
     * at the appropriate time, and that the computer does not retain a pointer to the memory after it has been
     * destroyed.
     *
     * The memory provided must not be null and must not be 0 bytes in size.
     *
     * @param memory
     */
    explicit Computer(MemoryType * memory)
    : m_memory(memory),
      m_memoryOwned(false)
    {
        assert(memory && 0 < memory->size());
    }

    /**
     * Destroy a Computer instance.
     */
    virtual ~Computer()
    {
        if (m_memoryOwned) {
            delete m_memory;
        }

        m_memory = nullptr;
        m_memoryOwned = false;
    }

    /**
     * Fetch a CPU from the computer.
     *
     * @param idx
     * @return
     */
    Cpu * cpu( int idx = 0 ) const
    {
        assert(0 <= idx && m_cpus.size() > idx);
        return m_cpus[idx];
    }

    /**
     * Fetch all the CPUs in the computer.
     *
     * @return
     */
    inline const std::vector<Cpu *> & cpus() const
    {
        return m_cpus;
    }

    /**
     * Fetch a mutable vector of all the CPUs in the computer.
     *
     * @return
     */
    inline std::vector<Cpu *> & cpus()
    {
        return m_cpus;
    }

    /**
     * Fetch the computer's memory.
     *
     * @return
     */
    inline MemoryType * memory() const
    {
        return m_memory;
    }

    /**
     * Fetch the size of the computer's memory.
     *
     * TODO consider altering the return type.
     *
     * @return
     */
    inline int memorySize() const
    {
        return m_memory->size();
    }

    /**
     * Add a CPU to the computer.
     *
     * The CPU is only borrowed by the computer, it is not owned. It is up to the caller to ensure it will be destroyed
     * at the appropriate time and that the computer does not retain a reference to a destroyed CPU.
     *
     * @param cpu
     */
    inline void addCpu(Cpu * cpu)
    {
        assert(cpu);
        m_cpus.push_back(cpu);
    }

    /**
     * Remove a CPU from the computer.
     *
     * It is safe to call this with a pointer to a CPU that the Computer does not have. In this case, the call is a
     * NOOP.
     *
     * @param cpu
     */
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
    inline void setMemory(MemoryType * memory)
    {
        if (m_memoryOwned) {
            delete m_memory;
            m_memoryOwned = false;
        }

        // TODO pass the memory to the CPUs
        m_memory = memory;
    }

    /**
     * Provide the computer with new memory that it owns.
     *
     * @param memory
     * @param size
     */
    inline void giveMemory(MemoryType * memory)
    {
        if (m_memoryOwned) {
            delete m_memory;
        }

        // TODO pass the memory to the CPUs
        m_memoryOwned = true;
        m_memory = memory;
    }

    /**
     * Reset the computer.
     *
     * Subclasses must implement this method. It is up to the subclass to determine what resetting entails, but it's
     * probably at least going to reset the CPU and clear the memory.
     */
    virtual void reset() = 0;

    /**
     * Execute a given number of instructions.
     *
     * Subclasses must implement this method. It is up to the subclass to determine what executing instructions means.
     * In most cases it will involve fetching instructions from memory based on a program counter and incrementing the
     * counter.
     */
    virtual void run(int instructionCount) = 0;

protected:
    using Cpus = std::vector<Cpu *>;

    Cpus m_cpus;
    MemoryType * m_memory;

private:
    bool m_memoryOwned;
};

#endif // COMPUTER_H
