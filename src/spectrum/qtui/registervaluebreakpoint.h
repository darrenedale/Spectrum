//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QTUI_REGISTERVALUEBREAKPOINT_H
#define SPECTRUM_QTUI_REGISTERVALUEBREAKPOINT_H

#include "registerbreakpoint.h"
#include "../../z80/types.h"

namespace Spectrum::QtUi
{
    class RegisterValueBreakpoint
    : public RegisterBreakpoint
    {
    private:
        using Register16 = ::Z80::Register16;
        using UnsignedWord = ::Z80::UnsignedWord ;

    public:
        RegisterValueBreakpoint(Register16 reg, UnsignedWord targetValue)
        : RegisterBreakpoint(reg),
          m_targetValue(targetValue)
        {}

        [[nodiscard]] std::string typeName() const override;
        [[nodiscard]] std::string conditionDescription() const override;

        [[nodiscard]] inline UnsignedWord targetValue() const
        {
            return m_targetValue;
        }

        bool operator==(const Breakpoint &) const override;
        bool check(const BaseSpectrum & spectrum) override;

    private:
        UnsignedWord m_targetValue;
    };
}

#endif //SPECTRUM_QTUI_REGISTERVALUEBREAKPOINT_H
