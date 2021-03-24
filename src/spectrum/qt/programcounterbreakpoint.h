//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_QT_PROGRAMCOUNTERBREAKPOINT_H
#define SPECTRUM_QT_PROGRAMCOUNTERBREAKPOINT_H

#include <QObject>

#include "../../z80/types.h"
#include "../spectrum48k.h"
#include "breakpoint.h"
#include "thread.h"

namespace Spectrum::Qt
{
    /**
     * Breakpoint that triggers when the PC of the Z80 reaches a particular address.
     */
    class ProgramCounterBreakpoint
    : public Breakpoint
    {
    Q_OBJECT

    public:
        ProgramCounterBreakpoint(Thread & thread, Z80::UnsignedWord address, QObject * parent = nullptr)
        : Breakpoint(thread, parent),
          m_address(address)
        {}

        [[nodiscard]] inline ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

        bool check(const BaseSpectrum & spectrum) override;

    private:
        using UnsignedWord = ::Z80::UnsignedWord;

        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_QT_PROGRAMCOUNTERBREAKPOINT_H
