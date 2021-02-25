#include <cstdint>

#include "z80interpreter.h"
#include "../z80.h"


int main(int argc, char * argv[])
{
	std::uint8_t ram[65536];
	Z80Interpreter::run(new Z80(ram, 65536));
	return 0;
}
