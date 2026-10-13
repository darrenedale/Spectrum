//
// Created by darren on 10/10/2026.
//

#ifndef SPECTRUM_IO_POKFILEWRITER_H
#define SPECTRUM_IO_POKFILEWRITER_H

#include <ostream>

namespace Spectrum
{
    class PokeDefinition;
}

namespace Spectrum::Io
{
    /**
     * Writer for files in .pok format.
     *
     * .pok files contain one or more pokes for a single Spectrum program. The format is documented at
     * https://worldofspectrum.org/faq/reference/formats.htm
     */
    class PokFileWriter
    {
        public:
            /**
             * Initialise a new writer with a given file name.
             *
             * The file will be opened if it exists and is writable.
             */
            explicit PokFileWriter(const std::string &);

            /**
             * Initialise a new writer with a given input stream.
             *
             * The stream is borrowed and must remain valid for the full lifetime of the writer.
             */
            explicit PokFileWriter(std::ostream &);

            /** PokFileWriters cannot be copied owing to possible issues of output stream ownership. */
            PokFileWriter(const PokFileWriter &) = delete;

            /**
             * PokFileWriters cannot be moved.
             *
             * Moving would be technically possible, but is not yet required.
             */
            PokFileWriter(PokFileWriter &&) = delete;

            /** PokFileWriters cannot be copied owing to possible issues of output stream ownership. */
            PokFileWriter & operator=(const PokFileWriter &) = delete;

            /**
             * PokFileWriters cannot be moved.
             *
             * Moving would be technically possible, but is not yet required.
             */
            PokFileWriter & operator=(PokFileWriter &&) = delete;

            /**
             * Destructor.
             *
             * If the writer owns its stream, the stream is destroyed, otherwise it remains valid. It is the client
             * code's responsibility to ensure that borrowed streams are destroyed at the appropriate time.
             */
            virtual ~PokFileWriter();

            [[nodiscard]]
            bool isOpen() const noexcept
            {
                return nullptr != m_out;
            }

            void close();

            void addPoke(const PokeDefinition &) noexcept;

        private:
            /** Flag indicating whether the output stream is owned or borrowed. */
            bool m_borrowedStream;

            /** The output stream. */
            std::ostream * m_out;
    };
} // Io::Spectrum

#endif //SPECTRUM_IO_POKEFILEWRITER_H
