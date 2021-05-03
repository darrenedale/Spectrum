//
// Created by darren on 01/05/2021.
//

#ifndef SPECTRUM_DEBUGGER_INTEGERMEMORYWATCH_H
#define SPECTRUM_DEBUGGER_INTEGERMEMORYWATCH_H

#include <bit>
#include <sstream>
#include <iomanip>
#include <string>
#include "integermemorywatchbase.h"
#include "../../util/endian.h"
#include "../../util/numeric.h"

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
    : public IntegerMemoryWatchBase
    {
    public:
        /**
         * Convenience alias for the storage type of the watch.
         */
        using ValueType = int_t;

        /**
         * Initialise a new integer memory watcher.
         *
         * @param memory
         * @param address
         */
        IntegerMemoryWatch(BaseSpectrum::MemoryType * memory, ::Z80::UnsignedWord address)
        : IntegerMemoryWatchBase(memory, address)
        {}

        /**
         * The number of bytes this watch is observing.
         *
         * This is the size of the integer type provided as a template argument.
         *
         * @return The size in bytes.
         */
        [[nodiscard]] constexpr WatchSize size() const override
        {
            return sizeof(int_t);
        }

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

                if constexpr (1 != sizeof(int_t)) {
                    if constexpr (::Z80::HostByteOrder != ::Z80::Z80ByteOrder) {
                        if (byteOrder() != std::endian::big) {
                            value = Util::swapByteOrder(value);
                        }
                    } else if (byteOrder() == std::endian::big) {
                        value = Util::swapByteOrder(value);
                    }
                }

                std::ostringstream out;

                switch (base()) {
                    case Base::Hex:
                        out << std::hex << std::showbase << std::setfill('0') << std::setw(sizeof(int_t) * 2) << Util::asNumeric(value);
                        break;

                    case Base::Decimal:
                        out << std::dec << Util::asNumeric(value);
                        break;

                    case Base::Octal:
                        out << std::oct << std::showbase << Util::asNumeric(value);
                        break;
                }

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
    };
}

#endif //SPECTRUM_DEBUGGER_INTEGERMEMORYWATCH_H
