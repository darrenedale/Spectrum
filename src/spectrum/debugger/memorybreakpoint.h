//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_MEMORYBREAKPOINT_H
#define SPECTRUM_DEBUGGER_MEMORYBREAKPOINT_H

#include "breakpoint.h"
#include "../../z80/types.h"

namespace Spectrum::Debugger
{
    class MemoryBreakpoint
    : public Breakpoint
    {
    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        explicit MemoryBreakpoint(UnsignedWord address)
        : Breakpoint(),
          m_address(address)
        {}

        [[nodiscard]] std::string typeName() const override = 0;
        [[nodiscard]] std::string conditionDescription() const override = 0;

        [[nodiscard]] inline ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

        bool operator==(const Breakpoint &) const override = 0;
        bool check(const BaseSpectrum & spectrum) override = 0;

    private:
        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_DEBUGGER_MEMORYBREAKPOINT_H
