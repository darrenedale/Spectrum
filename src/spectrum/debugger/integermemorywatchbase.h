//
// Created by darren on 01/05/2021.
//

#ifndef SPECTRUM_DEBUGGER_INTEGERMEMORYWATCHBASE_H
#define SPECTRUM_DEBUGGER_INTEGERMEMORYWATCHBASE_H

#include "memorywatch.h"
#include "../../util/endian.h"

namespace Spectrum::Debugger
{
    /**
     * Abstract base class for integer-based memory watches.
     */
    class IntegerMemoryWatchBase
    : public MemoryWatch
    {
    public:
        /**
         * Enumeration of supported display bases.
         */
        enum class Base
        {
            Hex = 0,
            Decimal,
            Octal,
            Binary,
        };

        /**
         * Convenience alias for the byte order of the watched value.
         */
        using ByteOrder = std::endian;

        IntegerMemoryWatchBase(BaseSpectrum::MemoryType * memory, ::Z80::UnsignedWord address)
        : MemoryWatch(memory, address),
          m_base(Base::Decimal),
          m_byteOrder(::Z80::Z80ByteOrder)
        {}

        /**
         * The number of bytes this watch is observing.
         *
         * This is the size of the integer type provided as a template argument.
         *
         * @return The size in bytes.
         */
        [[nodiscard]] constexpr WatchSize size() const override = 0;

        /**
         * Fetch the name of the watch type.
         *
         * The type is "Int [<bits>]" where <bits> is the number of bits in the int type.
         *
         * @return The type.
         */
        [[nodiscard]] std::string typeName() const override = 0;

        /**
         * Fetch the current display value for the watched memory.
         *
         * @return
         */
        [[nodiscard]] std::string displayValue() const override = 0;

        /**
         * Fetch the base in which numbers are displayed.
         *
         * @return The base.
         */
        [[nodiscard]] Base base() const
        {
            return m_base;
        }

        /**
         * Set the base in which the numbers will be displayed.
         *
         * @param base
         */
        void setBase(Base base)
        {
            m_base = base;
        }

        /**
         * Fetch the byte order to use when interpreting the watched memory as an int.
         *
         * @return The byte order.
         */
        [[nodiscard]] ByteOrder byteOrder() const
        {
            return m_byteOrder;
        }

        /**
         * Set the byte order to use when interpreting the watched memory as an int.
         *
         * @param order The byte order.
         */
        void setByteOrder(ByteOrder order)
        {
            m_byteOrder = order;
        }

    private:
        /**
         * The base in which to display the value.
         */
        Base m_base;

        /**
         * The byte order to use when interpreting the watched memory as an int.
         */
        ByteOrder m_byteOrder;
    };
}

#endif //SPECTRUM_DEBUGGER_INTEGERMEMORYWATCHBASE_H
