//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QT_MEMORYBREAKPOINT_H
#define SPECTRUM_QT_MEMORYBREAKPOINT_H

#include "breakpoint.h"
#include "../../z80/types.h"

namespace Spectrum::QtUi
{
    class MemoryBreakpoint
    : public Breakpoint
    {
    Q_OBJECT

    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        MemoryBreakpoint(Thread & thread, UnsignedWord address, QObject * parent = nullptr)
        : Breakpoint(thread, parent),
          m_address(address)
        {}

        [[nodiscard]] inline ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

        bool check(const BaseSpectrum & spectrum) override = 0;

    private:
        UnsignedWord m_address;
    };
}

#endif //SPECTRUM_QT_MEMORYBREAKPOINT_H
