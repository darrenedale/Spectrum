//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_PROGRAMCOUNTERBREAKPOINT_H
#define SPECTRUM_DEBUGGER_PROGRAMCOUNTERBREAKPOINT_H

#include <QObject>

#include "../../z80/types.h"
#include "../basespectrum.h"
#include "breakpoint.h"
#include "../qtui/thread.h"

namespace Spectrum::Debugger
{
    /**
     * Breakpoint that triggers when the PC of the Z80 reaches a particular address.
     */
    class ProgramCounterBreakpoint
    : public Breakpoint
    {
    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        explicit ProgramCounterBreakpoint(UnsignedWord address)
        : Breakpoint(),
          m_address(address)
        {}

        [[nodiscard]] std::string typeName() const override;
        [[nodiscard]] std::string conditionDescription() const override;

        [[nodiscard]] inline ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

        bool operator==(const Breakpoint &) const override;
        bool check(const BaseSpectrum & spectrum) override;

    private:
        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_DEBUGGER_PROGRAMCOUNTERBREAKPOINT_H
