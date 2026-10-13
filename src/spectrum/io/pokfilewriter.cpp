//
// Created by darren on 10/10/2026.
//

#include <fstream>

#include "pokfilereader.h"
#include "pokfilewriter.h"

#include "../../util/assert.h"

using namespace Spectrum::Io;

namespace
{
    void writePokeLine(std::ostream & out, const Spectrum::PokeDefinition::Poke & poke, bool isLast = false)
    {
        std::print(
            out,
            "{} {} {}",
            static_cast<char>(isLast ? PokFileReader::LineType::LastPoke : PokFileReader::LineType::Poke),
            8,                                      // bank
            poke.address                            // address
        );

        for (const auto & byte : poke.bytes) {
            std::print(out, " {}", byte.has_value() ? static_cast<std::uint16_t>(byte.value()) : 0x100);
        }

        std::println(out, " 0");
    }
}

PokFileWriter::PokFileWriter(const std::string & fileName)
: m_borrowedStream(false),
  m_out(new std::ofstream(fileName))
{}

PokFileWriter::PokFileWriter(std::ostream &stream)
: m_borrowedStream(true),
  m_out(&stream)
{}

PokFileWriter::~PokFileWriter()
{
    if (isOpen()) {
        close();
    }
}

void PokFileWriter::close()
{
    sp_assert(isOpen(), "PokFileWriter has already been closed, can't close again");
    std::println(*m_out, "{}", static_cast<char>(PokFileReader::LineType::Eof));

    if (!m_borrowedStream) {
        delete m_out;
    }

    m_out = nullptr;
    m_borrowedStream = false;
}

void PokFileWriter::addPoke(const PokeDefinition & pokeDefinition) noexcept
{
    sp_assert(isOpen(), "PokFileWriter has already been closed, can't add more pokes");
    std::println(*m_out, "{}{}", static_cast<char>(PokFileReader::LineType::Name), pokeDefinition.name());
    const auto & pokes = pokeDefinition.pokes();

    for (const auto & poke : std::ranges::subrange(pokes.cbegin(), pokes.cend() - 1)) {
        writePokeLine(*m_out, poke);
    }

    writePokeLine(*m_out, pokes.back(), true);
}
