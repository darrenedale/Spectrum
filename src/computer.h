#ifndef COMPUTER_H
#define COMPUTER_H

#include <algorithm>
#include <memory>
#include <cstdint>
#include <vector>
#include <type_traits>
#include <cassert>
#include "memory.h"

class Cpu;

/**
 * Concept describing types that can be used as computer native byte types.
 *
 * @tparam T The type.
 */
template<class T>
concept ComputerByte = std::is_integral_v<T>;

/**
 * Abstract base class for computers.
 *
 * @tparam byte_t The type for the native byte for the computer. Must satisfy the ComputerByte concept.
 */
template<ComputerByte byte_t = std::uint8_t>
class Computer
{
public:
    using ByteType = byte_t;
    template<class address_t = std::uint64_t, class size_t = std::uint64_t>
    using MemoryType = Memory<byte_t, address_t, size_t>;

    /**
     * Initialise a new Computer with pre-existing memory.
     *
     * The computer will only borrow the provided memory, it is up to the caller to ensure that the memory is destroyed at the appropriate time, and that the
     * computer does not retain a pointer to the memory after it has been destroyed.
     *
     * The memory provided must not be null and must not be 0 bytes in size.
     *
     * @param memory
     */
    explicit Computer(MemoryType<> * memory)
    : m_memory(memory),
      m_memoryOwned(false)
    {
        assert(m_memory && 0 < m_memory->addressableSize());
    }

    /**
     * Initialise a new Computer with pre-existing memory.
     *
     * The computer will take ownership of the provided memory.
     *
     * The memory provided must not be null and must not be 0 bytes in size.
     *
     * @param memory
     */
    explicit Computer(std::unique_ptr<MemoryType<>> memory)
    : m_memory(memory.release()),
      m_memoryOwned(true)
    {
        assert(m_memory && 0 < m_memory->addressableSize());
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
     * Don't call with an index for a CPU that does not exist. If in doubt, check cpus().size() first.
     *
     * @param idx
     * @return
     */
    Cpu * cpu(int idx = 0) const
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
        const auto pos = std::find(m_cpus.cbegin(), m_cpus.cend(), cpu);

        if (pos == m_cpus.cend()) {
            return;
        }

        m_cpus.erase(pos, pos);
    }

    /**
     * Fetch the computer's memory.
     *
     * @return
     */
    inline MemoryType<> * memory() const
    {
        return m_memory;
    }

    /**
     * Fetch the size of the computer's memory.
     *
     * @return The memory size.
     */
    inline typename MemoryType<>::Size memorySize() const
    {
        return m_memory->addressableSize();
    }

    /**
     * Provide the computer with new memory that it borrows.
     *
     * The memory must not be null and must not be 0 bytes in size.
     *
     * @param memory The memory to loan to the computer.
     */
    inline void setMemory(MemoryType<> * memory)
    {
        if (m_memoryOwned) {
            delete m_memory;
            m_memoryOwned = false;
        }

        m_memory = memory;
        assert(m_memory && 0 < m_memory->addressableSize());

        for (auto * cpu : cpus()) {
            cpu->setMemory(m_memory);
        }
    }

    /**
     * Provide the computer with new memory that it owns.
     *
     * The memory must not be null and must not be 0 bytes in size.
     *
     * @param memory The memory to give to the computer.
     */
    inline void setMemory(std::unique_ptr<MemoryType<>> memory)
    {
        if (m_memoryOwned) {
            delete m_memory;
        }

        m_memoryOwned = true;
        m_memory = memory.release();
        assert(m_memory && 0 < m_memory->addressableSize());

        for (auto * cpu : cpus()) {
            cpu->setMemory(m_memory);
        }
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
    /**
     * Check whether the computer owns the memory it is using.
     *
     * @return true if the memory is owned by the computer, false if it is just borrowed.
     */
    [[nodiscard]] bool ownsMemory() const
    {
        return m_memoryOwned;
    }

    /**
     * Convenience alias for the CPU storage type.
     */
    using Cpus = std::vector<Cpu *>;

    /**
     * The CPUs attached to the computer.
     */
    Cpus m_cpus;

    /**
     * The computer's memory.
     */
    MemoryType<> * m_memory;

private:
    /**
     * Flag indicating whether or not the computer owns the memory object.
     */
    bool m_memoryOwned;
};

#endif // COMPUTER_H
