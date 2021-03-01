//
// Created by darren on 27/02/2021.
//

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

#include "filereader.h"

using namespace Test::Z80;

FileReader::FileReader(std::string fileName)
: m_inStream(),
  m_inFileName(std::move(fileName))
{
}

FileReader::~FileReader()
{
    m_inStream.close();
}

std::optional<::Z80::UnsignedWord> FileReader::parseWord(char bytes[4])
{
    ::Z80::UnsignedWord word = 0;

    for (int idx = 0; idx < 4; ++idx) {
        if (!std::isxdigit(bytes[idx])) {
            // invalid hex value
            std::cerr << "Invalid hex digit in 16-bit word: " << bytes[0] << bytes[1] << bytes[2] << bytes[3] << "\n";
            return {};
        }

        auto digit = std::tolower(bytes[idx]);

        if ('0' <= digit && '9' >= digit) {
            word |= (digit - '0') << ((3 - idx) * 4);
        } else {
            word |= (10 + digit - 'a') << ((3 - idx) * 4);
        }
    }

    return word;
}

std::optional<::Z80::UnsignedByte> FileReader::parseByte(char bytes[2])
{
    ::Z80::UnsignedByte byte = 0;

    for (int idx = 0; idx < 2; ++idx) {
        if (!std::isxdigit(bytes[idx])) {
            // invalid hex value
            std::cerr << "Invalid hex digit in 8-bit byte: " << bytes[0] << bytes[1] << "\n";
            return {};
        }

        auto digit = std::tolower(bytes[idx]);

        if ('0' <= digit && '9' >= digit) {
            byte |= (digit - '0') << ((1 - idx) * 4);
        } else {
            byte |= (10 + digit - 'a') << ((1 - idx) * 4);
        }
    }

    return byte;
}

std::optional<bool> FileReader::parseFlag(char byte)
{
    switch (byte) {
        case '0':
            return false;

        case '1':
            return true;

        default:
            std::cerr << "Invalid flag character: " << byte << "\n";
            return {};
    }
}

std::optional<std::uint8_t> FileReader::parseInterruptMode(char byte)
{
    switch (byte) {
        case '0':
        case '1':
        case '2':
            return byte - '0';

        default:
            std::cerr << "Invalid interrupt mode: " << byte << "\n";
            return {};
    }
}

bool FileReader::open()
{
    if (isOpen()) {
        return true;
    }

    m_inStream.open(m_inFileName);
    return isOpen();
}

void FileReader::skipEmptyLines()
{
    assert(isOpen());

    while (!m_inStream.eof() && '\n' == m_inStream.peek()) {
        m_inStream.get();
    }
}

std::optional<std::string> FileReader::readLine()
{
    assert(isOpen());
    char buffer[4096];
    m_inStream.getline(buffer, 4096);

    if (m_inStream.fail()) {
        std::cerr << "Error reading line from stream\n";
        return {};
    }

    return {buffer};

}

std::optional<std::string> FileReader::readDescription()
{
    return readLine();
}

bool FileReader::readRegisterPairs(State & state)
{
    assert(isOpen());
    auto line = readLine();

    if (!line) {
        return false;
    }

    if (64 != line->length()) {
        std::cerr << "Invalid register pairs line\n";
        return false;
    }

    *line += ' ';
    std::istringstream in(*line);

    for (auto & property : {&state.af, &state.bc, &state.de, &state.hl, &state.afShadow, &state.bcShadow, &state.deShadow, &state.hlShadow, &state.ix, &state.iy, &state.sp, &state.pc, &state.memptr}) {
        char bytes[4];
        in.read(bytes, 4);

        if (4 != in.gcount()) {
            std::cerr << "Failed reading 16-bit word from register pairs line\n";
            return false;
        }

        if (' ' != in.get()) {
            std::cerr << "Invalid delimiter in register pairs line\n";
            return false;
        }

        auto word = parseWord(bytes);

        if (!word) {
            std::cerr << "Failed parsing value for register pair\n";
            return false;
        }

        *property = *word;
    }

    return true;
}

bool FileReader::readRegistersFlagsTStates(State & state, std::size_t & tStates)
{
    assert(isOpen());
    auto line = readLine();

    if (!line) {
        return false;
    }

    std::istringstream in(*line);

    for (auto & property : {&state.i, &state.r,}) {
        char bytes[2];
        in.read(bytes, 2);

        if (2 != in.gcount()) {
            std::cerr << "Failed reading 8-bit register value from registers/flags/t-states line\n";
            return false;
        }

        if (' ' != in.get()) {
            std::cerr << "Invalid delimiter in registers/flags/t-states line\n";
            return false;
        }

        auto byte = parseByte(bytes);

        if (!byte) {
            std::cerr << "Failed parsing value for 8-bit register\n";
            return false;
        }

        *property = *byte;
    }

    for (auto & property : {&state.iff1, &state.iff2,}) {
        char byte = in.get();

        if (' ' != in.get()) {
            std::cerr << "Invalid delimiter in registers/flags/t-states line\n";
            return false;
        }

        auto flag = parseFlag(byte);

        if (!flag) {
            std::cerr << "Failed parsing flag in registers/flags/t-states line\n";
            return false;
        }

        *property = *flag;
    }

    char byte = in.get();

    if (' ' != in.get()) {
        std::cerr << "Invalid delimiter in registers/flags/t-states line\n";
        return false;
    }

    auto im = parseInterruptMode(byte);

    if (!im) {
        std::cerr << "Failed parsing interrupt mode in registers/flags/t-states line\n";
        return false;
    }

    state.im = *im;

    byte = in.get();

    if (' ' != in.get()) {
        std::cerr << "Invalid delimiter in registers/flags/t-states line\n";
        return false;
    }

    auto flag = parseFlag(byte);

    if (!flag) {
        std::cerr << "Failed parsing halted flag in registers/flags/t-states line\n";
        return false;
    }

    state.halted = *flag;

    // TODO validate
    in >> tStates;

    if (in.fail()) {
        std::cerr << "Failed parsing t-states in registers/flags/t-states line\n";
        return false;
    }

    return true;
}

std::optional<std::vector<MemoryBlock>> FileReader::readMemoryBlocks()
{
    assert(isOpen());
    std::vector<MemoryBlock> memory;

    while ('-' != m_inStream.peek()) {
        auto block = readMemoryBlock();

        if (!block) {
            std::cerr << "Failed reading memory block\n";
            return {};
        }

        memory.push_back(std::move(*block));
    }

    if (memory.empty()) {
        // empty data block
        std::cerr << "No data in memory block\n";
        return {};
    }

    return std::move(memory);
}

std::optional<MemoryBlock> FileReader::readMemoryBlock()
{
    assert(isOpen());
    auto line = readLine();

    if (!line) {
        return {};
    }

    std::istringstream in(*line);
    char bytes[4];
    in.read(bytes, 4);

    if (' ' != in.get()) {
        std::cerr << "Invalid delimiter in memory block between address and first byte value\n";
        return {};
    }

    auto address = parseWord(bytes);

    if (!address) {
        std::cerr << "Failed parsing memory block address\n";
        return {};
    }

    std::vector<UnsignedByte> data;

    while (!in.eof()) {
        char bytes[2];
        in.read(bytes, 2);

        if (0 == std::strncmp("-1", bytes, 2)) {
            break;
        }

        if (' ' != in.get()) {
            // invalid byte delimiter
            std::cerr << "Invalid delimiter in memory block between byte values\n";
            return {};
        }

        auto byte = parseByte(bytes);

        if (!byte) {
            // invalid byte
            std::cerr << "Failed to parse byte value in memory block\n";
            return {};
        }

        data.push_back(*byte);
    }

    if (in.eof()) {
        // badly terminated data block
        std::cerr << "Invalid termination of memory block - is the terminating '-1' missing?\n";
        return {};
    }

    if (data.empty()) {
        // empty data block
        std::cerr << "Empty data block\n";
        return {};
    }

    return std::make_optional<MemoryBlock>({*address, std::move(data)});
}
