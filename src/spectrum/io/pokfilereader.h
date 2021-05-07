//
// Created by darren on 27/03/2021.
//

#ifndef SPECTRUM_POKFILEREADER_H
#define SPECTRUM_POKFILEREADER_H

#include <istream>
#include <optional>
#include "../pokedefinition.h"

namespace Spectrum::Io
{
    /**
     * Reader for files in .pok format.
     *
     * .pok files contain one or more pokes for a single Spectrum program. The format is documented at https://worldofspectrum.org/faq/reference/formats.htm
     */
    class PokFileReader
    {
    public:
        /**
         * Initialise a new reader with a given file name.
         *
         * The file will be opened if it exists and is readable.
         */
        explicit PokFileReader(const std::string &);

        /**
         * Initialise a new reader with a given input stream.
         *
         * The stream is borrowed and must remain valid for the full lifetime of the reader.
         */
        explicit PokFileReader(std::istream &);

        /**
         * PokFileReaders cannot be copied owing to possible issues of input stream ownership.
         */
        PokFileReader(const PokFileReader &) = delete;

        /**
         * PokFileReaders cannot be moved.
         *
         * Moving would be technically possible, but is not yet required.
         */
        PokFileReader(PokFileReader &&) = delete;

        /**
         * PokFileReaders cannot be copied owing to possible issues of input stream ownership.
         */
        void operator=(const PokFileReader &) = delete;

        /**
         * PokFileReaders cannot be moved.
         *
         * Moving would be technically possible, but is not yet required.
         */
        void operator=(PokFileReader &&) = delete;

        /**
         * Destructor.
         *
         * If the reader owns its stream, the stream is destroyed, otherwise it remains valid. It is the client code's responsibililty to ensure that borrowed
         * streams are destroyed at the appropriate time.
         */
        virtual ~PokFileReader();

        /**
         * Check the reader is valid.
         *
         * @return true if the input stream can be read, false if not.
         */
        [[nodiscard]] bool isValid() const
        {
            return !m_in->bad();
        }

        /**
         * Check whether the input stream contains any more poke definitions.
         *
         * @return true if there are more to read, false otherwise.
         */
        [[nodiscard]] bool hasMorePokes() const;

        /**
         * Read the next poke from the input stream.
         *
         * @return The poke, or an empty optional if there are no more pokes to read or an error occurred.
         */
        std::optional<PokeDefinition> nextPoke();

    protected:
        /**
         * Enumeration of the types of line in a .pok file.
         */
        enum class LineType
        {
            Unrecognised,
            Name,
            Poke,
            LastPoke,
            Eof,
        };

        /**
         * Helper to determine the type of the next line in the input stream.
         *
         * The stream must be positioned at the start of a line, otherwise the result is undefined.
         *
         * @return The line type.
         */
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

        /**
         * Helper to parse a line read from the input stream into a poke definition.
         *
         * @param line The line to parse.
         *
         * @return The parsed poke, or an empty optional if the line cannot be parsed.
         */
        static std::optional<PokeDefinition::Poke> parsePokeLine(const std::string & line);

        /**
         * Helper to read a line from the input stream.
         *
         * @return The read line.
         */
        inline std::string readLine()
        {
            std::string line;
            std::getline(*m_in, line);
            return line;
        }

    private:
        /**
         * Flag indicating whether the input stream is owned or borrowed.
         */
        bool m_borrowedStream;

        /**
         * The input stream.
         */
        std::istream * m_in;
    };
}

#endif //SPECTRUM_POKFILEREADER_H
