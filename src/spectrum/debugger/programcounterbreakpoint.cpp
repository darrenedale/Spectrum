//
// Created by darren on 12/03/2021.
//

#include <sstream>
#include <iomanip>

#include "programcounterbreakpoint.h"

using namespace Spectrum::Debugger;

bool ProgramCounterBreakpoint::check(const BaseSpectrum & spectrum)
{
    if (spectrum.z80()->pc() == m_address) {
        notifyObservers();
        return true;
    }

    return false;
}

bool ProgramCounterBreakpoint::operator==(const Breakpoint & other) const
{
    return typeid(*this) == typeid(other) && address() == dynamic_cast<const ProgramCounterBreakpoint *>(&other)->address();
}

std::string ProgramCounterBreakpoint::typeName() const
{
    return "Program counter";
}

std::string ProgramCounterBreakpoint::conditionDescription() const
{
    std::ostringstream out;
    out << "PC == 0x" << std::hex << std::setfill('0') << std::setw(4) << address();
    return out.str();
}
