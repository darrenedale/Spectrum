//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_MEMORYBREAKPOINT_H
#define SPECTRUM_DEBUGGER_MEMORYBREAKPOINT_H

#include "breakpoint.h"
#include "../../z80/types.h"

namespace Spectrum::Debugger
{
    /**
     * A breakpoint base class that triggers according to conditions on a given memory address.
     *
     * The condition is up to the subclass to define and implement. For example, a subclass might monitor an address for when a byte/word value at that address
     * changes, or when it reaches a given value.
     */
    class MemoryBreakpoint
    : public Breakpoint
    {
    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        /**
         * Initialise a new MemoryBreakpoint with a given address.
         *
         * @param address The address that is the subject of the breakpoint.
         */
        explicit MemoryBreakpoint(UnsignedWord address)
        : Breakpoint(),
          m_address(address)
        {}

        /**
         * Fetch the address that is the subject of the breakpoint.
         *
         * @return The address.
         */
        [[nodiscard]] inline UnsignedWord address() const
        {
            return m_address;
        }

        /**
         * Set the address that is the subject of the breakpoint.
         *
         * It is the caller's responsibility to ensure that the address is valid for the Spectrum object they intend to check it against.
         *
         * Reimplement this if there is additional housekeeping that needs to be done when the address is set. Call this base class implementation in your
         * reimplementation in order to ensure that address() continues to provide the correct address.
         *
         * @param address
         */
        inline virtual void setAddress(UnsignedWord address)
        {
            m_address = address;
        }

    private:
        /**
         * The address that is the subject of the breakpoint.
         */
        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_DEBUGGER_MEMORYBREAKPOINT_H
