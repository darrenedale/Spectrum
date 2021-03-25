#include "cpu.h"

Cpu::Cpu(MemoryType * memory)
: m_memory(memory),
  m_clockSpeed(0)
{}

Cpu::Cpu(Cpu && other) noexcept
: m_memory(other.m_memory),
  m_clockSpeed(other.m_clockSpeed)
{
    other.m_memory = nullptr;
}


Cpu & Cpu::operator=(Cpu && other) noexcept
{
    m_memory = other.m_memory;
    m_clockSpeed = other.m_clockSpeed;
    other.m_memory = nullptr;
    return *this;
}

Cpu::~Cpu() = default;

void Cpu::setMemory(MemoryType * memory)
{
    m_memory = memory;
}
