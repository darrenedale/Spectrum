//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QT_WORDCHANGEDBREAKPOINT_H
#define SPECTRUM_QT_WORDCHANGEDBREAKPOINT_H

#include "memorybreakpoint.h"

namespace Spectrum::QtUi
{
    /**
     * Breakpoint that monitors a specific memory location for a change in its value.
     *
     * The monitored location can be checked for a change to any size of int starting at that location.
     */
    class WordChangedBreakpoint
    : public MemoryBreakpoint
    {
        Q_OBJECT

    private:
        using UnsignedWord = ::Z80::UnsignedWord;

    public:
        using MemoryBreakpoint::MemoryBreakpoint;

        bool check(const BaseSpectrum & spectrum) override;

    private:
        std::optional<UnsignedWord> m_lastSeenValue;
    };
}

#endif //SPECTRUM_QT_WORDCHANGEDBREAKPOINT_H
