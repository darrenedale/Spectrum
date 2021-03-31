//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QTUI_REGISTERBREAKPOINT_H
#define SPECTRUM_QTUI_REGISTERBREAKPOINT_H

#include "breakpoint.h"
#include "../../z80/types.h"

namespace Spectrum::QtUi
{
    class RegisterBreakpoint
    : public Breakpoint
    {
    private:
        using Register16 = ::Z80::Register16;

    public:
        explicit RegisterBreakpoint(Register16 reg)
        : Breakpoint(),
          m_register(reg)
        {}

        [[nodiscard]] std::string typeName() const override = 0;
        [[nodiscard]] std::string conditionDescription() const override = 0;

        [[nodiscard]] inline Register16 watchedRegister() const
        {
            return m_register;
        }

        bool operator==(const Breakpoint &) const override = 0;
        bool check(const BaseSpectrum & spectrum) override = 0;

    private:
        Register16 m_register;
    };
}

#endif //SPECTRUM_QTUI_REGISTERBREAKPOINT_H
