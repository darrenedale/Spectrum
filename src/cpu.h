#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include "memory.h"

/**
 * Abstraction of an emulated CPU.
 */
class Cpu
{
public:
    /**
     * Alias for the base type of memory objects for the CPU.
     */
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

    /**
     * Destructor.
     */
    virtual ~Cpu();

    /**
     * The memory is borrowed, not owned (multiple CPUs could be addressing the same memory). The caller is responsible
     * for destroying the memory when it is no longer needed, and ensuring that CPU no longer keeps a reference to it
     * once it has been discarded.
     *
     * @param memory
     * @param memorySize
     */
    void setMemory(MemoryType * memory);

    /**
     * Fetch a read-write pointero to the memory the CPU is using.
     *
     * The memory could be nullptr.
     *
     * @return A pointer to the memory.
     */
    inline MemoryType * memory()
    {
        return m_memory;
    }

    /**
     * Fetch a read-only pointer to the memory the CPU is using.
     *
     * The memory could be nullptr.
     *
     * @return A pointer to the memory.
     */
    [[nodiscard]] inline const MemoryType * memory() const
    {
        return m_memory;
    }

    /**
     * Fetch the addressable size of the CPU's memory.
     *
     * Do not call this on a CPU without memory. If you're unsure, check isValid() or memory() first.
     *
     * @return The addressable size.
     */
    [[nodiscard]] inline int memorySize() const
    {
        return static_cast<int>(m_memory->addressableSize());
    }

    /**
     * Set the notional clock speed of the CPU.
     *
     * This has no impact on the actual speed of the emulation - the consumer of the CPU can, however, use this setting
     * to determine timings that keep the CPU appearing to run at this speed.
     *
     * @param hz The speed of the CPU in hertz (cycles per second).
     */
    inline void setClockSpeed(unsigned long long hz)
    {
        m_clockSpeed = hz;
    }

    /**
     * Set the notional clock speed of the CPU in megahertz.
     *
     * This has no impact on the actual speed of the emulation - the consumer of the CPU can, however, use this setting
     * to determine timings that keep the CPU appearing to run at this speed.
     *
     * @param hz The speed of the CPU in megahertz hertz (millions of cycles per second).
     */
    inline void setClockSpeedMHz(double mhz)
    {
        m_clockSpeed = static_cast<unsigned long long>(mhz * 1000000);
    }

    /**
     * Fetch the clock speed of the CPU.
     *
     * @return The clock speed in hertz (cycles per second).
     */
    [[nodiscard]] inline unsigned long long clockSpeed() const
    {
        return m_clockSpeed;
    }

    /**
     * Fetch the clock speed of the CPU.
     *
     * @return The clock speed in megahertz (millions of cycles per second).
     */
    [[nodiscard]] inline double clockSpeedMHz() const
    {
        return static_cast<double>(m_clockSpeed) / 1000000.0;
    }

protected:
    /**
     * The CPU's memory.
     *
     * This is borrowed, not owned.
     */
    MemoryType * m_memory;

    /**
     * The notional clock speed of the CPU.
     */
    unsigned long long m_clockSpeed;
};

#endif // CPU_H
