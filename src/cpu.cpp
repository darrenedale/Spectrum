#include "cpu.h"

Cpu::Cpu(unsigned char * memory, int memorySize)
: m_memory(memory),
  m_memorySize(memorySize),
  m_clockSpeed(0)
{}

Cpu::~Cpu() = default;

void Cpu::setMemory(unsigned char * memory, int memorySize)
{
    m_memory = memory;
    m_memorySize = memorySize;
}
