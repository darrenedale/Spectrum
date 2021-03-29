//
// Created by darren on 29/03/2021.
//

#include "bytechangedbreakpoint.h"

using namespace Spectrum::QtUi;

bool ByteChangedBreakpoint::check(const BaseSpectrum & spectrum)
{
    assert(address() <= spectrum.memorySize() - 1);
    auto currentValue = spectrum.memory()->readByte(address());
    bool hit = m_lastSeenValue && *m_lastSeenValue != currentValue;
    m_lastSeenValue = currentValue;

    if (hit) {
        trigger();
    }

    return hit;
}
