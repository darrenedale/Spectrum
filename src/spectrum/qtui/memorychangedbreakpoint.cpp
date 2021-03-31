//
// Created by darren on 29/03/2021.
//

#include "memorychangedbreakpoint.h"

//using namespace Spectrum::QtUi;
//
//bool WordChangedBreakpoint::check(const BaseSpectrum & spectrum)
//{
//    assert(address() <= spectrum.memorySize() - 2);
//    auto currentValue = spectrum.memory()->readWord<UnsignedWord>(address());
//    bool hit = m_lastSeenValue && *m_lastSeenValue != currentValue;
//    m_lastSeenValue = currentValue;
//
//    if (hit) {
//        notifyObservers();
//    }
//
//    return hit;
//}
//
//bool WordChangedBreakpoint::operator==(const Breakpoint & other) const
//{
//    return typeid(*this) == typeid(other) && address() == dynamic_cast<const WordChangedBreakpoint *>(&other)->address();
//}
