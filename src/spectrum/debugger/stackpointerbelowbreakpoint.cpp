//
// Created by darren on 31/03/2021.
//

#include <iomanip>
#include <sstream>
#include <string>

#include "stackpointerbelowbreakpoint.h"

using namespace std::string_literals;

using namespace Spectrum::Debugger;

std::string StackPointerBelowBreakpoint::typeName() const noexcept
{
    return "Stack pointer below"s;
}

std::string StackPointerBelowBreakpoint::conditionDescription() const
{
    std::ostringstream out;
    out << "SP < 0x" << std::hex << std::setfill('0') << std::setw(4) << address();
    return out.str();
}

bool StackPointerBelowBreakpoint::operator==(const Breakpoint & other) const noexcept
{
    return typeid(*this) == typeid(other) && address() == reinterpret_cast<const StackPointerBelowBreakpoint *>(&other)->address();
}

bool StackPointerBelowBreakpoint::check(const BaseSpectrum & spectrum) noexcept
{
    if (spectrum.z80()->stackPointer() < address()) {
        notifyObservers();
        return true;
    }

    return false;
}
