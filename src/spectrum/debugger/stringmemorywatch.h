//
// Created by darren on 01/05/2021.
//

#ifndef SPECTRUM_DEBUGGER_STRINGMEMORYWATCH_H
#define SPECTRUM_DEBUGGER_STRINGMEMORYWATCH_H

#include <string>
#include <iostream>
#include "memorywatch.h"
#include "../../z80/types.h"

namespace Spectrum::Debugger
{
    /**
     * Watch the value of a chunk of memory as a string value.
     *
     * The watched memory can be interpreted either according to the Spectrum standard character set (which differs slightly from ASCII) or as ASCII. Other
     * encodings may follow. The display value is always returned as the UTF-8 encoding of the interpreted string. Invalid characters are replaced with
     * U+FFFD (Replacement Character).
     *
     * @tparam Size The number of bytes to watch.
     */
    class StringMemoryWatch
    : public MemoryWatch
    {
    public:
        /**
         * Enumeration of supported display charsets.
         */
        enum class CharacterEncoding
        {
            Spectrum = 0,
            Ascii,
        };

        /**
         * Initialise a new StringMemoryWatch with a given memory object, address and string length.
         *
         * The watched string must fall entirely within the addressable size of the provided memory.
         *
         * @param memory The memory to watch.
         * @param address The address to watch in the memory.
         * @param size The size of the string to watch.
         */
        StringMemoryWatch(BaseSpectrum::MemoryType * memory, ::Z80::UnsignedWord address, WatchSize size);

        /**
         * Fetch the name of the watch type.
         *
         * The type is "String [<char size>]" where <char size> is the number of bytes in the string.
         *
         * @return The type name.
         */
        [[nodiscard]] std::string typeName() const override;

        /**
         * Fetch the size of the string being watched.
         *
         * The size is in bytes.
         *
         * @return The size.
         */
        [[nodiscard]] WatchSize size() const override
        {
            return m_size;
        }

        /**
         * Set the size of the string being watched.
         *
         * @param size The string size.
         */
        void setSize(WatchSize size)
        {
            if (m_size != size) {
                m_size = size;
                m_typeName.clear();
            }
        }

        /**
         * Fetch the character encoding to assume for the watched memory bytes.
         *
         * @return The current character encoding.
         */
        [[nodiscard]] CharacterEncoding characterEncoding() const
        {
            return m_charset;
        }

        /**
         * Set the character encoding to assume for the watched memory bytes.
         *
         * @param encoding The encoding to use.
         */
        void setCharacterEncoding(CharacterEncoding encoding)
        {
            m_charset = encoding;
        }

        /**
         * Fetch the string at the watched address.
         *
         * The watched bytes in memory are retrieved and interpreted according to the specified encoding. The returned string is UTF-8 encoded.
         *
         * @return The display string.
         */
        [[nodiscard]] std::string displayValue() const override;

    protected:
        /**
         * Helper to append the "character" from the Spectrum character set for a given index.
         *
         * @param out The output stream to write to.
         * @param ch The character index.
         */
        static void appendSpectrumChar(std::ostream & out, ::Z80::UnsignedByte ch);

        /**
         * The size in bytes of the string being watched.
         */
        WatchSize m_size;

        /**
         * The character encoding to use when interpreting the watched bytes.
         */
        CharacterEncoding m_charset;

        /**
         * Cache for the type name.
         *
         * Emptied when the size is set; set when the type name is requested and it's not currently set.
         */
        mutable std::string m_typeName;
    };
}

#endif //SPECTRUM_DEBUGGER_STRINGMEMORYWATCH_H
