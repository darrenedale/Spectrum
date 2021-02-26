#ifndef CPU_H
#define CPU_H

#include <cstdint>

class Cpu
{
public:
    Cpu(std::uint8_t * mem, int memSize);
    virtual ~Cpu();

    void setMemory(std::uint8_t * mem, int memsize);

    inline std::uint8_t * memory()
    {
        return m_ram;
    }

    inline int memorySize() const
    {
        return m_ramSize;
    }

    inline void setClockSpeed(unsigned long long hz)
    {
        m_clockSpeed = hz;
    }

    inline void setClockSpeedMHz(double mhz)
    {
        m_clockSpeed = static_cast<unsigned long long>(mhz * 1000000);
    }

    inline unsigned long long clockSpeed() const
    {
        return m_clockSpeed;
    }

    inline double clockSpeedMHz() const
    {
        return static_cast<double>(m_clockSpeed) / 1000000.0;
    }

protected:
    std::uint8_t * m_ram;
    int m_ramSize;
    unsigned long long m_clockSpeed;
};

#endif // CPU_H
