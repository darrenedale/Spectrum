//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_QT_PROGRAMCOUNTERBREAKPOINT_H
#define SPECTRUM_QT_PROGRAMCOUNTERBREAKPOINT_H

#include <QObject>

#include "../../z80/types.h"
#include "../basespectrum.h"
#include "breakpoint.h"
#include "thread.h"

namespace Spectrum::QtUi
{
    /**
     * Breakpoint that triggers when the PC of the Z80 reaches a particular address.
     */
    class ProgramCounterBreakpoint
    : public Breakpoint
    {
    Q_OBJECT
    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        ProgramCounterBreakpoint(Thread & thread, UnsignedWord address, QObject * parent = nullptr)
        : Breakpoint(thread, parent),
          m_address(address)
        {}

        [[nodiscard]] inline ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

        bool check(const BaseSpectrum & spectrum) override;

    private:
        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_QT_PROGRAMCOUNTERBREAKPOINT_H
