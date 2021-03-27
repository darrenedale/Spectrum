//
// Created by darren on 27/03/2021.
//

#ifndef SPECTRUM_POKFILEREADER_H
#define SPECTRUM_POKFILEREADER_H

#include <istream>
#include <optional>
#include "pokedefinition.h"

namespace Spectrum
{
    class PokFileReader
    {
    public:
        explicit PokFileReader(const std::string &);
        explicit PokFileReader(std::istream &);
        PokFileReader(const PokFileReader &) = delete;
        PokFileReader(PokFileReader &&) = delete;
        void operator=(const PokFileReader &) = delete;
        void operator=(PokFileReader &&) = delete;
        virtual ~PokFileReader();

        [[nodiscard]] bool isValid() const
        {
            return !m_in->bad();
        }

        [[nodiscard]] bool hasMorePokes() const;
        std::optional<PokeDefinition> nextPoke();

    protected:
        enum class LineType : std::uint8_t
        {
            Unrecognised,
            Name,
            Poke,
            LastPoke,
            Eof,
        };

        [[nodiscard]] LineType nextLineType() const
        {
            switch ((*m_in).peek()) {
                case 'N':
                    return LineType::Name;

                case 'M':
                    return LineType::Poke;

                case 'Z':
                    return LineType::LastPoke;

                case 'Y':
                    return LineType::Eof;
            }

            return LineType::Unrecognised;
        }

        std::optional<PokeDefinition::Poke> parsePokeLine(const std::string & line);

        inline std::string readLine()
        {
            std::string line;
            std::getline(*m_in, line);
            return line;
        }

    private:
        bool m_borrowedStream;
        std::istream * m_in;
    };
}

#endif //SPECTRUM_POKFILEREADER_H
