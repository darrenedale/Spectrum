//
// Created by darren on 29/03/2021.
//

#include "wordchangedbreakpoint.h"

using namespace Spectrum::QtUi;

bool Spectrum::QtUi::WordChangedBreakpoint::check(const BaseSpectrum & spectrum)
{
    assert(address() <= spectrum.memorySize() - 2);
    auto currentValue = spectrum.memory()->readWord<UnsignedWord>(address());
    bool hit = m_lastSeenValue && *m_lastSeenValue != currentValue;
    m_lastSeenValue = currentValue;

    if (hit) {
        trigger();
    }

    return hit;
}
