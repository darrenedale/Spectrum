#ifndef CPU_H
#define CPU_H

#include <cstdint>

class Cpu
{
public:
    /**
     * Memory is borrowed, not owned. Caller is responsible for deallocating memory when no longer needed, and ensuring
     * that CPU no longer keeps a reference to the memory once it has been discarded.
     *
     * @param memory
     * @param memorySize
     */
    Cpu(std::uint8_t * memory, int memorySize);
    virtual ~Cpu();

    /**
     * Memory is borrowed, not owned. Caller is responsible for deallocating memory when no longer needed, and ensuring
     * that CPU no longer keeps a reference to the memory once it has been discarded.
     *
     * @param memory
     * @param memorySize
     */
    void setMemory(std::uint8_t * memory, int memorySize);

    inline std::uint8_t * memory()
    {
        return m_memory;
    }

    [[nodiscard]] inline int memorySize() const
    {
        return m_memorySize;
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
    std::uint8_t * m_memory;
    int m_memorySize;
    unsigned long long m_clockSpeed;
};

#endif // CPU_H
