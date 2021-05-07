//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_REGISTERBREAKPOINT_H
#define SPECTRUM_DEBUGGER_REGISTERBREAKPOINT_H

#include "breakpoint.h"
#include "../../z80/types.h"

namespace Spectrum::Debugger
{
    /**
     * Base class for breakpoints that monitor the state of 16-bit register pairs.
     */
    class RegisterBreakpoint
    : public Breakpoint
    {
    protected:
        /**
         * Convenience alias for the register type.
         */
        using Register16 = ::Z80::Register16;

    public:
        /**
         * Initialise a new RegisterBreakpoint for a given 16-bit register pair.
         *
         * @param reg The register that is the subject of the breakpoint.
         */
        explicit RegisterBreakpoint(Register16 reg)
        : Breakpoint(),
          m_register(reg)
        {}

        /**
         * Fetch the register being watched by this breakpoint.
         *
         * NOTE this method name deviates from the usual naming convention because register is a reserved word in C++.
         *
         * @return The register.
         */
        [[nodiscard]] inline Register16 watchedRegister() const
        {
            return m_register;
        }

    private:
        /**
         * The register that is the subject of the breakpoint.
         */
        Register16 m_register;
    };
}

#endif //SPECTRUM_DEBUGGER_REGISTERBREAKPOINT_H
