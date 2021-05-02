//
// Created by darren on 01/05/2021.
//

#ifndef SPECTRUM_STRINGMEMORYWATCH_H
#define SPECTRUM_STRINGMEMORYWATCH_H

#include <sstream>
#include "memorywatch.h"
#include "../../z80/types.h"

namespace Spectrum::Debugger
{
    /**
     * Watch the value of a chunk of memory as a string value.
     *
     * Currently, the value is always represented as a string of characters in the Spectrum character set.
     *
     * @tparam Size The number of bytes to watch.
     *
     * TODO support switching between ASCII/Spectrum charset?
     */
    class StringMemoryWatch
    : public MemoryWatch
    {
    public:
        using StringSize = std::uint32_t;

        StringMemoryWatch(BaseSpectrum::MemoryType * memory, ::Z80::UnsignedWord address, StringSize size)
        : MemoryWatch(memory, address),
          m_size(static_cast<std::uint32_t>(size))
        {}

        /**
         * Fetch the name of the watch type.
         *
         * The type is "String [<char size>]" where <char size> is the number of bytes in the string.
         *
         * @return The type name.
         */
        [[nodiscard]] std::string typeName() const override
        {
            // TODO consider caching this in a member, clearing it when size is set, to avoid repeatedly building the string
            std::ostringstream out;
            out << "String [" << size() << ']';
            return out.str();
        }

        /**
         * Fetch the size of the string being watched.
         *
         * The size is in bytes.
         *
         * @return The size.
         */
        [[nodiscard]] StringSize size() const
        {
            return m_size;
        }

        /**
         * Set the size of the string being watched.
         *
         * @param size The string size.
         */
        void setSize(StringSize size)
        {
            m_size = size;
        }

        /**
         * Fetch the current display value for the watched memory.
         *
         * @return The display value.
         */
        [[nodiscard]] std::string displayValue() const override
        {
            std::array<::Z80::UnsignedByte, size()> str;
            memory()->readBytes(address(), str.size(), std.data());
            std::ostringstream out;

            for (const auto & byte : str) {
                appendSpectrumChar(out, byte);
            }

            return out.str();
        }

    protected:
        /**
         * Helper to append the "character" from the Spectrum character set for a given index.
         *
         * @param out The output stream to write to.
         * @param ch The character index.
         */
        static void appendSpectrumChar(std::ostream & out, ::z80::UnsignedByte ch)
        {
            // keyword "characters" from 165 - 255
            static std::array<std::string, 91> keywords = {
                "[RND]"s, "[INKEY$]"s, "[PI]"s, "[FN]"s, "[POINT]"s,
                "[SCREEN$]"s, "[ATTR]"s, "[AT]"s, "[TAB]"s, "[VAL$]"s, "[CODE]"s, "[VAL]"s, "[LEN]"s, "[SIN]"s, "[COS]"s,
                "[TAN]"s, "[ASN]"s, "[ACS]"s, "[ATN]"s, "[LN]"s, "[EXP]"s, "[INT]"s, "[SQR]"s, "[SGN]"s, "[ABS]"s,
                "[PEEK]"s, "[IN]"s, "[USR]"s, "[STR$]"s, "[CHR$]"s, "[NOT]"s, "[BIN]"s, "[OR]"s, "[AND]"s, "[<=]"s,
                "[>=]"s, "[<>]"s, "[LINE]"s, "[THEN]"s, "[TO]"s, "[STEP]"s, "[DEF FN]"s, "[CAT]"s, "[FORMAT]"s, "[MOVE]"s,
                "[ERASE]"s, "[OPEN #]"s, "[CLOSE #]"s, "[MERGE]"s, "[VERIFY]"s, "[BEEP]"s, "[CIRCLE]"s, "[INK]"s, "[PAPER]"s, "[FLASH]"s,
                "[BRIGHT]"s, "[INVERSE]"s, "[OVER]"s, "[OUT]"s, "[LPRINT]"s, "[LLIST]"s, "[STOP]"s, "[READ]"s, "[DATA]"s, "[RESTORE]"s,
                "[NEW]"s, "[BORDER]"s, "[CONTINUE]"s, "[DIM]"s, "[REM]"s, "[FOR]"s, "[GO TO]"s, "[GO SUB]"s, "[INPUT]"s, "[LOAD]"s,
                "[LIST]"s, "[LET]"s, "[PAUSE]"s, "[NEXT]"s, "[POKE]"s, "[PRINT]"s, "[PLOT]"s, "[RUN]"s, "[SAVE]"s, "[RANDOMIZE]"s,
                "[IF]"s, "[CLS]"s, "[DRAW]"s, "[CLEAR]"s, "[RETURN]"s, "[COPY]"s,
            };

            // TODO 128 to 143 are block graphics chars
            if (96 == ch) {
                // UK pounds sterling symbol
                out << '£';
            } else if (127 == ch) {
                // copyright symbol
                out << "©"; // U+00A9 Copyright Sign (0xc2 0xa9 in UTF-8)
            } else if (32 <= ch && 127 >= ch) {
                // other characters from 32 - 127 inclusive are as ASCII
                out << static_cast<std::ostream::char_type>(ch);
            } else if (12 == ch) {
                // delete
                out << "⌫"; // U+232b Erase to the Left (0xe2 0x8c 0xab in UTF-8)
            } else if (13 == ch) {
                // enter
                out << "⏎"; // U+23ce Return Symbol (0xe2 0x8f 0x8e in UTF-8)
            } else if (165 <= ch) {
                // Spectrum BASIC keyword chars
                out << keywords[ch - 165];
            } else {
                out << "�"; // U+FFFD Replacement Character (0xef 0xbf 0xbd in UTF-8)
            }
        }

        /**
         * The size in bytes of the string being watched.
         */
        StringSize m_size;
    };
}

#endif //SPECTRUM_STRINGMEMORYWATCH_H
