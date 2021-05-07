//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_STACKPOINTERBELOWBREAKPOINT_H
#define SPECTRUM_DEBUGGER_STACKPOINTERBELOWBREAKPOINT_H

#include "breakpoint.h"
#include "../../z80/types.h"
#include "../basespectrum.h"

namespace Spectrum::Debugger
{
    /**
     * Breakpoint that triggers when the SP of the Z80 is below a particular address.
     */
    class StackPointerBelowBreakpoint
    : public Breakpoint
    {
    private:
        /**
         * Convenience alias for the type of the trigger SP address.
         */
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        /**
         * Initialise a new StackPointerBelowBreakpoint with a given address.
         *
         * @param address The address below which the Sp should trigger the breakpoint.
         */
        explicit StackPointerBelowBreakpoint(UnsignedWord address)
        : Breakpoint(),
          m_address(address)
        {}

        /**
         * The type name for this type of breakpoint.
         *
         * @return "Stack pointer below"
         */
        [[nodiscard]] std::string typeName() const override;

        /**
         * A human-readable description of the breakpoint's condition.
         *
         * @return "SP <= 0x<value>"
         */
        [[nodiscard]] std::string conditionDescription() const override;

        /**
         * The address against which the SP is being checked.
         *
         * @return The address.
         */
        [[nodiscard]] inline ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

        /**
         * Check whether two breakpoints are equivalent.
         *
         * In order to be considered equivalent, the other breakpoint must be of the same type as this and be monitoring the SP against the same address.
         *
         * @return true if the breakpoints are equivalent, false otherwise.
         */
        bool operator==(const Breakpoint &) const override;

        /**
         * Check the given Spectrum object state for whether it matches the breakpoint condition.
         *
         * If the condition is met all observers are notified.
         *
         * @param spectrum The Spectrum to check.
         *
         * @return true if the Spectrum state meets the breakpoint condition, false otherwise.
         */
        bool check(const BaseSpectrum & spectrum) override;

    private:
        /**
         * The address below which SP triggers the breakpoint.
         */
        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_DEBUGGER_STACKPOINTERBELOWBREAKPOINT_H
