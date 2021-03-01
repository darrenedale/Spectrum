//
// Created by darren on 26/02/2021.
//

#include <cassert>
#include <iostream>
#include <sstream>
#include <cstring>

#include "testfilereader.h"

using namespace Test::Z80;
using TestClass = ::Test::Z80::Test;

TestFileReader::TestFileReader(std::string fileName)
: FileReader(std::move(fileName))
{
}

std::optional<TestClass> TestFileReader::nextTest()
{
    if (!isOpen()) {
        std::cerr << "Failed reading test - reader is not open\n";
        return {};
    }

    skipEmptyLines();
    auto name = readName();

    if (!name) {
        // failed to read name
        std::cerr << "Failed reading name for test\n";
        return {};
    }

    State state;

    if (!readRegisterPairs(state)) {
        std::cerr << "Failed reading register pairs for test " << *name << "\n";
        return {};
    }

    std::size_t tStates;

    if (!readRegistersFlagsTStates(state, tStates)) {
        std::cerr << "Failed reading registers, flags and t-states for test " << *name << "\n";
        return {};
    }

    auto memory = readMemoryBlocks();

    if (!memory) {
        std::cerr << "Failed reading memory block for test " << *name << "\n";
        return {};
    }

    state.memory = std::move(*memory);

    if (!readEndOfTestMarker()) {
        std::cerr << "Failed reading end of test marker for test " << *name << "\n";
        return {};
    }

    skipEmptyLines();
    return Test(std::move(*name), tStates, std::move(state));
}

bool TestFileReader::readEndOfTestMarker()
{
    assert(isOpen());
    auto line = readLine();
    return line && "-1" == *line;
}
