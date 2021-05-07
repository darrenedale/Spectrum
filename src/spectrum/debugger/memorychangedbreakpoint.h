//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_MEMORYCHANGEDBREAKPOINT_H
#define SPECTRUM_DEBUGGER_MEMORYCHANGEDBREAKPOINT_H

#include <sstream>
#include <iomanip>
#include "memorybreakpoint.h"

namespace Spectrum::Debugger
{
    template<class int_t>
    concept MemoryChangedBreakpointValueType = std::is_integral_v<int_t>;

    /**
     * Breakpoint that monitors a specific memory location for a change in its value.
     *
     * The monitored location can be checked for a change to any size of int starting at that location.
     */
    template<MemoryChangedBreakpointValueType value_t>
    class MemoryChangedBreakpoint
    : public MemoryBreakpoint
    {
    public:
        /**
         * Import base class constructor(s).
         */
        using MemoryBreakpoint::MemoryBreakpoint;

        /**
         * The name of this type of breakpoint.
         *
         * @return "Memory value change"
         */
        [[nodiscard]] std::string typeName() const override
        {
            return "Memory value change";
        }

        /**
         * A human-readable description of the breakpoint's condition.
         *
         * @return "<bit-size>-bit value at address 0x<address> changes"
         */
        [[nodiscard]] std::string conditionDescription() const override
        {
            std::ostringstream out;
            out << (sizeof(value_t) * 8) << "-bit value at address 0x" << std::hex << std::setfill('0') << std::setw(4) << address() << " changes";
            return out.str();
        }

        /**
         * Check whether two breakpoints are equivalent.
         *
         * In order to be considered equivalent, the other breakpoint must be of the same type as this and be monitoring the same address.
         *
         * @param other
         *
         * @return True if the two breakpoints are equivalent, false otherwise.
         */
        bool operator==(const Breakpoint & other) const override
        {
            return typeid(*this) == typeid(other) && address() == dynamic_cast<const MemoryChangedBreakpoint<value_t> *>(&other)->address();
        }

        /**
         * Check whether the memory address in the given Spectrum's memory has changed since it was last checked.
         *
         * The provided Spectrum object must have some memory. If the memory at the address has changed, all observers are notified.
         *
         * @param spectrum The spectrum to check.
         *
         * @return true if the memory value has changed, false if not.
         */
        bool check(const BaseSpectrum & spectrum) override
        {
            auto * memory = spectrum.memory();
            assert(memory && address() <= memory->addressableSize() - sizeof(value_t));
            auto currentValue = memory->readWord<value_t>(address());

            // NOTE the breakpoint never triggers on the first check since we don't know what the memory value was before the breakpoint was created.
            bool changed = m_lastSeenValue && *m_lastSeenValue != currentValue;
            m_lastSeenValue = currentValue;

            if (changed) {
                notifyObservers();
            }

            return changed;
        }

    private:
        /**
         * The last value seen at the memory address being monitored.
         */
        std::optional<value_t> m_lastSeenValue;
    };
}

#endif //SPECTRUM_DEBUGGER_MEMORYCHANGEDBREAKPOINT_H
