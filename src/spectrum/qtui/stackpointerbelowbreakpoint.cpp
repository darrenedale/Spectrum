//
// Created by darren on 31/03/2021.
//

#include <sstream>
#include <iomanip>
#include "stackpointerbelowbreakpoint.h"

using namespace Spectrum::QtUi;

std::string StackPointerBelowBreakpoint::typeName() const
{
    return "Stack pointer below";
}

std::string StackPointerBelowBreakpoint::conditionDescription() const
{
    std::ostringstream out;
    out << "SP < 0x" << std::hex << std::setfill('0') << std::setw(4) << address();
    return out.str();
}

bool StackPointerBelowBreakpoint::operator==(const Breakpoint & other) const
{
    return typeid(*this) == typeid(other) && address() == reinterpret_cast<const StackPointerBelowBreakpoint *>(&other)->address();
}

bool StackPointerBelowBreakpoint::check(const Spectrum::BaseSpectrum & spectrum)
{
    if (spectrum.z80()->stackPointer() < address()) {
        notifyObservers();
        return true;
    }

    return false;
}
