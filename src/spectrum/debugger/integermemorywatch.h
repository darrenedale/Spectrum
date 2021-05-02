//
// Created by darren on 01/05/2021.
//

#ifndef SPECTRUM_DEBUGGER_NUMERICMEMORYWATCH_H
#define SPECTRUM_DEBUGGER_NUMERICMEMORYWATCH_H

#include <bit>
#include <sstream>
#include <iomanip>
#include <string>
#include "../../util/endian.h"
#include "../../util/numeric.h"
#include "memorywatch.h"

namespace Spectrum::Debugger
{
    template<class T>
    concept IntegerMemoryWatchType = std::is_integral_v<T>;

    /**
     * Watch a chunk of memory as if it represents an int type.
     *
     * @tparam int_t The int type to use when reading the value from memory.
     */
    template<IntegerMemoryWatchType int_t>
    class IntegerMemoryWatch
    : public MemoryWatch
    {
    public:
        /**
         * Enumeration of supported display bases.
         */
        enum class Base
        {
            Hex = 0,
            Decimal = 1,
            Octal = 2,
        };

        /**
         * Convenience alias for the storage type of the watch.
         */
        using ValueType = int_t;

        /**
         * Convenience alias for the byte order of the watched value.
         */
        using ByteOrder = std::endian;

        /**
         * Initialise a new integer memory watcher.
         *
         * @param memory
         * @param address
         */
        IntegerMemoryWatch(BaseSpectrum::MemoryType * memory, ::Z80::UnsignedWord address)
        : MemoryWatch(memory, address),
          m_base(Base::Decimal),
          m_byteOrder(::Z80::Z80ByteOrder)
        {}

        /**
         * Fetch the name of the watch type.
         *
         * The type is "Int [<bits>]" where <bits> is the number of bits in the int type.
         *
         * @return The type.
         */
        [[nodiscard]] std::string typeName() const override
        {
            static std::unique_ptr<std::string> name = nullptr;

            if (!name) {
                std::ostringstream out;
                out << "Int [" << (sizeof(int_t) * 8) << ']';
                name = std::make_unique<std::string>(out.str());
            }

            return *name;
        }

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

        /**
         * Fetch the current display value for the watched memory.
         *
         * @return
         */
        [[nodiscard]] std::string displayValue() const
        {
            if constexpr (8 < sizeof(int_t)) {
                return bigDisplayValue();
            } else {
                auto value = memory()->template readWord<int_t>(address());

                Util::debug << (8 * sizeof(int_t)) << "-bit value @ " << std::hex << std::showbase << std::setfill('0') << std::setw(4) << address() << " is " << Util::asNumeric(value) << std::dec << std::setfill(' ') << '\n';

                if constexpr (1 != sizeof(int_t)) {
                    // TODO check this logic
                    if ((byteOrder() == std::endian::native) != (byteOrder() == ::Z80::Z80ByteOrder)) {
                        Util::swapByteOrder(value);
                    }
                }

                std::ostringstream out;

                switch (base()) {
                    case Base::Hex:
                        out << std::hex << std::showbase << std::setfill('0') << std::setw(sizeof(int_t) / 4) << Util::asNumeric(value);
                        break;

                    case Base::Decimal:
                        out << std::dec << Util::asNumeric(value);
                        break;

                    case Base::Octal:
                        out << std::oct << std::showbase << Util::asNumeric(value);
                        break;

                    default:
                        Util::debug << "invalid base - empty string\n";
                        break;
                }

                Util::debug << "Returning \"" << out.str() << "\"\n";
                return out.str();
            }
        }

    protected:
        /**
         * Helper to fetch the display value for ints of size > 64 bits.
         *
         * Currently only hex display is supported.
         *
         * @return The display value.
         */
        [[nodiscard]] std::string bigDisplayValue() const
        {
            std::array<BaseSpectrum::MemoryType::Byte, sizeof(int_t)> buffer;
            memory()->readBytes(address(), sizeof(int_t), buffer.data());

            std::ostringstream out;

            switch (base()) {
                case Base::Decimal:
                    return "<bigint decimal display not available>";

                case Base::Hex:
                    if (byteOrder() != ::Z80::Z80ByteOrder) {
                        std::reverse(std::begin(buffer), std::end(buffer));
                    }

                    out << std::hex << std::setfill('0') << "0x";

                    for (const auto & byte : buffer) {
                        out << std::setw(2) << static_cast<std::uint16_t>(byte);
                    }
                    break;

                case Base::Octal:
                    return "<bigint octal display not available>";
            }

            return out.str();
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

#endif //SPECTRUM_DEBUGGER_NUMERICMEMORYWATCH_H
