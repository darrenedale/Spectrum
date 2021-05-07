//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_REGISTERVALUEBREAKPOINT_H
#define SPECTRUM_DEBUGGER_REGISTERVALUEBREAKPOINT_H

#include "registerbreakpoint.h"
#include "../../z80/types.h"

namespace Spectrum::Debugger
{
    /**
     * Breakpoint that monitors a 16-bit register pair and triggers when it changes to a target value.
     */
    class RegisterValueBreakpoint
    : public RegisterBreakpoint
    {
    private:
        /**
         * Convenience alias for the type of the target value.
         */
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        /**
         * Initialise a new RegisterValueBreakpoint for a given 16-bit register pair and target value.
         *
         * @param reg The register to monitor.
         * @param targetValue The target value.
         */
        RegisterValueBreakpoint(Register16 reg, UnsignedWord targetValue)
        : RegisterBreakpoint(reg),
          m_targetValue(targetValue)
        {}

        /**
         * The type name for this type of breakpoint.
         *
         * @return "Register pair value"
         */
        [[nodiscard]] std::string typeName() const override;

        /**
         * A human-readable description of the breakpoint's condition.
         *
         * @return <register-name> == 0x<value>
         */
        [[nodiscard]] std::string conditionDescription() const override;

        /**
         * Fetch the target value for the register.
         *
         * @return The value.
         */
        [[nodiscard]] inline UnsignedWord targetValue() const
        {
            return m_targetValue;
        }

        /**
         * Check whether two breakpoints are equivalent.
         *
         * In order to be considered equivalent, the other breakpoint must be of the same type as this and be monitoring the same register pair for the same
         * value.
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
         * The target value for the register.
         */
        UnsignedWord m_targetValue;
    };
}

#endif //SPECTRUM_DEBUGGER_REGISTERVALUEBREAKPOINT_H
