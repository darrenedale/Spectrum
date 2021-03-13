//
// Created by darren on 12/03/2021.
//

#include <iostream>
#include <iomanip>

#include "programcounterbreakpoint.h"

using namespace Spectrum::Qt;

bool ProgramCounterBreakpoint::check(const ::Spectrum::Spectrum & spectrum)
{
    if (spectrum.z80()->pc() == m_address) {
        trigger();
        return true;
    }

    return false;
}
