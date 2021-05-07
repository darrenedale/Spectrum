//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_PROGRAMCOUNTERBREAKPOINT_H
#define SPECTRUM_DEBUGGER_PROGRAMCOUNTERBREAKPOINT_H

#include "breakpoint.h"
#include "../../z80/types.h"
#include "../basespectrum.h"

namespace Spectrum::Debugger
{
    /**
     * Breakpoint that triggers when the PC of the Z80 reaches a particular address.
     */
    class ProgramCounterBreakpoint
    : public Breakpoint
    {
    private:
        /**
         * Convenience alias for Z80 addresses.
         */
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        /**
         * Initialise a new ProgramCounterBreakpoint with a given address.
         *
         * @param address The address at which to break when the PC reaches it.
         */
        explicit ProgramCounterBreakpoint(UnsignedWord address)
        : Breakpoint(),
          m_address(address)
        {}

        /**
         * The type name for breakpoints of this class.
         *
         * @return "Program counter"
         */
        [[nodiscard]] std::string typeName() const override;

        /**
         * A human-readable description of the condition implemented by this breakpoint.
         *
         * @return "PC == 0x<address>"
         */
        [[nodiscard]] std::string conditionDescription() const override;

        /**
         * The address against which the PC is checked.
         *
         * @return The address.
         */
        [[nodiscard]] inline UnsignedWord address() const
        {
            return m_address;
        }

        /**
         * Check whether two breakpoints are equivalent.
         *
         * In order to be considered equivalent, the other breakpoint must be of the same type as this and be monitoring PC for the same address.
         *
         * @return true if the two breakpoints are equivalent, false otherwise.
         */
        bool operator==(const Breakpoint &) const override;

        /**
         * Check the given Spectrum object state to see whether its PC has reached the address.
         *
         * @param spectrum The spectrum to check.
         *
         * @return true if the Spectrum's PC is at the address, false otherwise.
         */
        bool check(const BaseSpectrum & spectrum) override;

    private:
        /**
         * The address to check against the PC.
         */
        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_DEBUGGER_PROGRAMCOUNTERBREAKPOINT_H
