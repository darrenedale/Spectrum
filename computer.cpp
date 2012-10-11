#include "computer.h"

#include <cstdlib>


Computer::Computer( int memsize )
:	m_ram(0),
	m_ramSize(0),
	m_myRam(true) {
	m_ram = (unsigned char *) std::malloc(sizeof(unsigned char) * memsize);
	if(m_ram) m_ramSize = memsize;
}


Computer::Computer( unsigned char * mem, int memsize )
:	m_ram(mem),
	m_ramSize(memsize),
	m_myRam(false) {}


Computer::~Computer( void ) {
	if(m_myRam && m_ram) std::free(m_ram);
	m_ram = 0;
	m_ramSize = 0;
}


Cpu * Computer::cpu( int i ) {
	try {
		return m_cpus.item(i);
	}
	catch(int e) {
		return 0;
	}
}
