//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QTUI_MEMORYCHANGEDBREAKPOINT_H
#define SPECTRUM_QTUI_MEMORYCHANGEDBREAKPOINT_H

#include <sstream>
#include <iomanip>
#include "memorybreakpoint.h"

namespace Spectrum::QtUi
{
    /**
     * Breakpoint that monitors a specific memory location for a change in its value.
     *
     * The monitored location can be checked for a change to any size of int starting at that location.
     */
    template<class ValueType>
    class MemoryChangedBreakpoint
    : public MemoryBreakpoint
    {
    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        using MemoryBreakpoint::MemoryBreakpoint;

        [[nodiscard]] std::string typeName() const override
        {
            return "Memory value change";
        }

        [[nodiscard]] std::string conditionDescription() const override
        {
            std::ostringstream out;
            out << (sizeof(ValueType) * 8) << "-bit value at address 0x" << std::hex << std::setfill('0') << std::setw(4) << address() << " changes";
            return out.str();
        }

        bool operator==(const Breakpoint & other) const override
        {
            return typeid(*this) == typeid(other) && address() == dynamic_cast<const MemoryChangedBreakpoint<ValueType> *>(&other)->address();
        }

        bool check(const BaseSpectrum & spectrum) override
        {
            assert(address() <= spectrum.memorySize() - 2);
            auto currentValue = spectrum.memory()->readWord<ValueType>(address());
            bool hit = m_lastSeenValue && *m_lastSeenValue != currentValue;
            m_lastSeenValue = currentValue;

            if (hit) {
                notifyObservers();
            }

            return hit;
        }

    private:
        std::optional<ValueType> m_lastSeenValue;
    };
}

#endif //SPECTRUM_QTUI_MEMORYCHANGEDBREAKPOINT_H
