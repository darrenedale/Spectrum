#include <cstdint>

#include "z80interpreter.h"
#include "../z80/z80.h"

using namespace Interpreter;

int main(int argc, char * argv[])
{
	std::uint8_t ram[65536];
	Z80Interpreter::run(new Z80::Z80(ram, 65536));
	return 0;
}
