//
// Created by darren on 26/02/2021.
//

#include <cassert>

#include "testfilereader.h"

using namespace Spectrum::Test::Z80;

TestFileReader::TestFileReader(std::FILE * inFile)
: m_inFile(inFile),
  m_borrowedFile(true)
{
}

TestFileReader::TestFileReader(std::string fileName)
: m_inFile(nullptr),
  m_borrowedFile(false),
  m_inFileName(std::move(fileName))
{
}

TestFileReader::~TestFileReader()
{
    if (!m_borrowedFile && m_inFile) {
        std::fclose(m_inFile);
        m_inFile = nullptr;
    }
}

std::optional<Test> TestFileReader::nextTest() const
{
    if (!isOpen()) {
        return {};
    }

    skipEmptyLines();
    auto description = readDescription();

    if (!description) {
        // failed to read description
        return {};
    }

    InitialState state;

    if (!readRegisters(state)) {
        return {};
    }

    if (!readFlags(state)) {
        return {};
    }

    std::size_t len;
    char buffer[20];
    state.tStates = std::stoul(fgets(buffer, sizeof(buffer) / sizeof(char), m_inFile), &len);

    auto memory = readMemoryBlocks();

    if (!memory) {
        return {};
    }

    state.memory = std::move(*memory);
    skipEmptyLines();
    return Test(std::move(*description), std::move(state));
}

std::optional<::Z80::UnsignedWord> TestFileReader::readWord() const
{
    assert(isOpen());
    std::uint8_t bytes[6] = {0, 0, 0, 0, 0, 0};
    auto bytesRead = std::fread(bytes, sizeof(std::uint8_t), 5, m_inFile);

    if (5 != bytesRead) {
        // not enough bytes read
        return {};
    }

    if (' ' != bytes[4] && '\n' != bytes[4]) {
        // not terminated by whitespace/LF
        return {};
    }

    ::Z80::UnsignedWord word = 0;

    for (int idx = 0; idx < 4; ++idx) {
        if (!std::isxdigit(bytes[idx])) {
            // invalid hex value
            return {};
        }

        auto digit = std::tolower(bytes[idx]);

        if ('0' <= digit && '9' >= digit) {
            word |= (digit - '0') << ((3 - idx) * 4);
        } else {
            word |= (digit - 'a') << ((3 - idx) * 4);
        }
    }

    return word;
}

std::optional<::Z80::UnsignedByte> TestFileReader::readByte() const
{
    assert(isOpen());
    std::uint8_t bytes[4] = {0, 0, 0, 0,};
    auto bytesRead = std::fread(bytes, sizeof(std::uint8_t), 3, m_inFile);

    if (3 != bytesRead) {
        // not enough bytes read
        return {};
    }

    if (' ' != bytes[2] && '\n' != bytes[2]) {
        // not terminated by whitespace/LF
        return {};
    }

    ::Z80::UnsignedByte byte = 0;

    for (int idx = 0; idx < 2; ++idx) {
        if (!std::isxdigit(bytes[idx])) {
            // invalid hex value
            return {};
        }

        auto digit = std::tolower(bytes[idx]);

        if ('0' <= digit && '9' >= digit) {
            byte |= (digit - '0') << ((1 - idx) * 4);
        } else {
            byte |= (digit - 'a') << ((1 - idx) * 4);
        }
    }

    return byte;
}

std::optional<bool> TestFileReader::readFlag() const
{
    assert(isOpen());
    std::uint8_t bytes[3] = {0, 0, 0,};
    auto bytesRead = std::fread(bytes, sizeof(std::uint8_t), 2, m_inFile);

    if (2 != bytesRead) {
        // not enough bytes read
        return {};
    }

    if (' ' != bytes[1] && '\n' != bytes[1]) {
        // not terminated by whitespace/LF
        return {};
    }

    switch (bytes[0]) {
        case '0':
            return false;

        case '1':
            return true;

        default:
            return {};
    }
}

bool TestFileReader::open()
{
    if (isOpen()) {
        return true;
    }

    m_inFile = std::fopen(m_inFileName.c_str(), "rb");
    return isOpen();
}

std::optional<std::string> TestFileReader::readDescription() const
{
    char buffer[4096];
    std::string description;

    do {
        std::fgets(buffer, 4096, m_inFile);
        description += buffer;
    } while (!feof(m_inFile) && '\n' != description[description.length() - 1]);

    if ('\n' != description[description.length() - 1]) {
        return {};
    }

    // trim off trailing linefeed
    description[description.length() - 1] = 0;
    return description;
}

bool TestFileReader::readRegisters(InitialState & state) const
{
    for (auto & property : {&state.af, &state.bc, &state.de, &state.hl, &state.afShadow, &state.bcShadow, &state.deShadow, &state.hlShadow, &state.ix, &state.iy, &state.sp, &state.pc, &state.memptr}) {
        auto word = readWord();

        if (!word) {
            return false;
        }

        *property = *word;
    }

    for (auto & property : {&state.i, &state.r}) {
        auto byte = readByte();

        if (!byte) {
            return {};
        }

        *property = *byte;
    }

    return true;
}

bool TestFileReader::readFlags(InitialState & state) const
{
    for (auto & property : {&state.iff1, &state.iff2, &state.im, &state.halted,}) {
        auto flag = readFlag();

        if (!flag) {
            return false;
        }

        *property = *flag;
    }

    return true;
}

void TestFileReader::skipEmptyLines() const
{
    assert(isOpen());

    while (!std::feof(m_inFile) && '\n' == std::fgetc(m_inFile)) {}

    if (!std::feof(m_inFile)) {
        std::fseek(m_inFile, -1, SEEK_CUR);
    }
}

std::optional<std::vector<MemoryBlock>> TestFileReader::readMemoryBlocks() const
{
    std::vector<MemoryBlock> memory;

    while ('-' != std::fgetc(m_inFile)) {
        std::fseek(m_inFile, -1, SEEK_CUR);
        auto block = readMemoryBlock();

        if (!block) {
            return {};
        }

        memory.push_back(std::move(*block));
    }

    if ('1' != std::fgetc(m_inFile) || '\n' != std::fgetc(m_inFile)) {
        // badly terminated data block
        return {};
    }

    if (memory.empty()) {
        // empty data block
        return {};
    }

    return std::move(memory);
}

std::optional<MemoryBlock> TestFileReader::readMemoryBlock() const
{
    auto address = readWord();

    if (!address) {
        return {};
    }

    std::vector<UnsignedByte> data;

    while ('-' != std::fgetc(m_inFile)) {
        std::fseek(m_inFile, -1, SEEK_CUR);
        auto byte = readByte();

        if (!byte) {
            return {};
        }

        data.push_back(*byte);
    }

    if ('1' != std::fgetc(m_inFile) || '\n' != std::fgetc(m_inFile)) {
        // badly terminated data block
        return {};
    }

    if (data.empty()) {
        // empty data block
        return {};
    }

    return std::make_optional<MemoryBlock>({*address, std::move(data)});
}
