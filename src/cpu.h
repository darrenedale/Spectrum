#ifndef CPU_H
#define CPU_H

#include <cstdint>

#include "memory.h"

class Cpu
{
public:
    using MemoryType = Memory<std::uint8_t>;

    /**
     * Memory is borrowed, not owned. Caller is responsible for deallocating memory when no longer needed, and ensuring
     * that CPU no longer keeps a reference to the memory once it has been discarded.
     *
     * @param memory
     * @param memorySize
     */
    explicit Cpu(MemoryType * memory);

    /**
     * Note that if you copy construct a CPU object, you create another CPU that is sharing the memory of the original,
     * so the owner of that memory needs to know about it. In general copying CPUs is best avoided as it's likely to
     * make your code complicated and error-prone.
     */
    Cpu(const Cpu &) = default;

    /**
     * Note that if you move construct a CPU object, you create a CPU that is sharing the memory of the original and the
     * original CPU ceases to be a consumer of the memory, both of which the owner of that memory needs to know about.
     * In general moving CPUs is best avoided as it's likely to make your code complicated and error-prone.
     */
    Cpu(Cpu &&) noexcept;

    /**
     * Note that if you copy a CPU object, you create another CPU that is sharing the memory of the original, so the
     * owner of that memory needs to know about it. In general copying CPUs is best avoided as it's likely to make
     * your code complicated and error-prone.
     */
    Cpu & operator=(const Cpu &) = default;

    /**
     * Note that if you move a CPU object, you create a CPU that is sharing the memory of the original and the original
     * CPU ceases to be a consumer of the memory, both of which the owner of that memory needs to know about. In general
     * moving CPUs is best avoided as it's likely to make your code complicated and error-prone.
     */
    Cpu & operator=(Cpu &&) noexcept;

    virtual ~Cpu();

    /**
     * Memory is borrowed, not owned. Caller is responsible for deallocating memory when no longer needed, and ensuring
     * that CPU no longer keeps a reference to the memory once it has been discarded.
     *
     * @param memory
     * @param memorySize
     */
    void setMemory(MemoryType * memory);

    inline MemoryType * memory()
    {
        return m_memory;
    }

    [[nodiscard]] inline int memorySize() const
    {
        return m_memory->addressableSize();
    }

    inline void setClockSpeed(unsigned long long hz)
    {
        m_clockSpeed = hz;
    }

    inline void setClockSpeedMHz(double mhz)
    {
        m_clockSpeed = static_cast<unsigned long long>(mhz * 1000000);
    }

    [[nodiscard]] inline unsigned long long clockSpeed() const
    {
        return m_clockSpeed;
    }

    [[nodiscard]] inline double clockSpeedMHz() const
    {
        return static_cast<double>(m_clockSpeed) / 1000000.0;
    }

protected:
    MemoryType * m_memory;
    unsigned long long m_clockSpeed;
};

#endif // CPU_H
