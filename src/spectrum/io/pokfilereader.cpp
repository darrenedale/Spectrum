//
// Created by darren on 27/03/2021.
//

#include <fstream>
#include <sstream>
#include "pokfilereader.h"
#include "../../util/debug.h"

using namespace Spectrum::Io;

using Spectrum::PokeDefinition;
using Poke = Spectrum::PokeDefinition::Poke;

namespace
{
    /**
     * Helper to skip over whitespace when reading from a stream.
     *
     * @param in The stream to skip.
     */
    void skipWhitespace(std::istream & in)
    {
        while (std::isspace(in.peek())) {
            in.get();
        }
    }
}

PokFileReader::PokFileReader(const std::string & fileName)
: m_in(new std::ifstream(fileName)),
  m_borrowedStream(false)
{}

PokFileReader::PokFileReader(std::istream & inFile)
: m_in(&inFile),
  m_borrowedStream(true)
{
}

PokFileReader::~PokFileReader()
{
    if (!m_borrowedStream) {
        delete m_in;
    }

    m_in = nullptr;
    m_borrowedStream = false;
}

bool PokFileReader::hasMorePokes() const
{
    return isValid() && !m_in->eof() && LineType::Eof != nextLineType();
}

std::optional<Spectrum::PokeDefinition> PokFileReader::nextPoke()
{
    if (!hasMorePokes()) {
        return {};
    }

    if (LineType::Name != nextLineType()) {
        Util::debug << "Unexpected line type: found '" << static_cast<char>((*m_in).peek()) << "' expecting 'N'.";
        return {};
    }

    PokeDefinition poke;
    std::string line = readLine();
    poke.setName({line.cbegin() + 1, line.cend()});

    while (true) {
        auto lineType = nextLineType();

        if (lineType != LineType::Poke && lineType != LineType::LastPoke) {
            Util::debug << "Unexpected line type: found: '" << static_cast<char>((*m_in).peek()) << "' expecting 'M' or 'Z'.";
            return {};
        }

        auto pokeValue = parsePokeLine(readLine());

        if (!pokeValue) {
            Util::debug << "Error parsing poke line '" << line << "'\n";
            return {};
        }

        poke.addPoke(std::move(*pokeValue));

        if (LineType::LastPoke == lineType) {
            break;
        }
    }

    return poke;
}

std::optional<Poke> PokFileReader::parsePokeLine(const std::string & line)
{
    assert(!line.empty() && ('M' == line[0] || 'Z' == line[0]));
    PokeDefinition::Poke poke;

    std::istringstream in(line);

    // skip the line type character
    in.get();
    skipWhitespace(in);

    // read and skip bank
    ::Z80::UnsignedWord value;
    in >> value;

    if (in.fail()) {
        Util::debug << "failed reading poke address\n";
        return {};
    }

    skipWhitespace(in);

    // read poke address
    in >> poke.address;

    if (in.fail()) {
        Util::debug << "failed reading poke address\n";
        return {};
    }

    skipWhitespace(in);

    // read poke value
    // NOTE we read it as a word rather than a byte so that we get the parsed value rather than the ASCII code of the
    // next character in the line
    in >> value;

    if (in.fail() || value > 0x100) {
        Util::debug << "failed reading poke value\n";
        return {};
    } else if (value == 0x100) {
        // 0x100 means "user-provided value", which in a Poke object is represented by an unfilled optional
        poke.bytes = {{}};
    } else {
        poke.bytes = {static_cast<::Z80::UnsignedByte>(value)};
    }

    skipWhitespace(in);

    // read undo value and skip it, just for validation
    in >> value;

    if (in.fail() || value > 0xff) {
        Util::debug << "failed reading poke undo value\n";
        return {};
    }

    return poke;
}
