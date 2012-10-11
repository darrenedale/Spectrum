#include "cpu.h"

Cpu::Cpu( unsigned char * mem, int memsize )
:	m_ram(mem),
	m_ramSize(memsize) {}


Cpu::~Cpu( void ) {}


void Cpu::setMemory( unsigned char * mem, int memsize ) {
	m_ram = mem;
	m_ramSize = memsize;
}
