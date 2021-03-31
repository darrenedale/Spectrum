//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_QTUI_STACKPOINTERBELOWBREAKPOINT_H
#define SPECTRUM_QTUI_STACKPOINTERBELOWBREAKPOINT_H

#include <QObject>

#include "../../z80/types.h"
#include "../basespectrum.h"
#include "breakpoint.h"
#include "thread.h"

namespace Spectrum::QtUi
{
    /**
     * Breakpoint that triggers when the SP of the Z80 is below a particular address.
     */
    class StackPointerBelowBreakpoint
    : public Breakpoint
    {
    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        explicit StackPointerBelowBreakpoint(UnsignedWord address)
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

#endif //SPECTRUM_QTUI_STACKPOINTERBELOWBREAKPOINT_H
