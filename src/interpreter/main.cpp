#include <cstdint>

#include "z80interpreter.h"
#include "../z80/z80.h"
#include "../simplememory.h"

using namespace Interpreter;

int main(int argc, char * argv[])
{
    using Memory = SimpleMemory<::Z80::UnsignedByte>;
	const auto memory = std::make_unique<Memory>(65536);
	Z80Interpreter::run(std::make_unique<Z80::Z80>(memory.get()));
	return 0;
}
