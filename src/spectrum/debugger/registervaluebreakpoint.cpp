//
// Created by darren on 31/03/2021.
//

#include <iomanip>
#include <sstream>
#include <string>

#include "../basespectrum.h"
#include "registervaluebreakpoint.h"

using namespace std::string_literals;

using namespace Spectrum::Debugger;

bool RegisterValueBreakpoint::operator==(const Breakpoint & other) const noexcept
{
    if (typeid(*this) != typeid(other)) {
        return false;
    }

    const auto & rvOther = *reinterpret_cast<const RegisterValueBreakpoint *>(&other);
    return watchedRegister() == rvOther.watchedRegister() && targetValue() == rvOther.targetValue();
}

bool RegisterValueBreakpoint::check(const BaseSpectrum & spectrum) noexcept
{
    if (spectrum.z80()->registerValue(watchedRegister()) == targetValue()) {
        notifyObservers();
        return true;
    }

    return false;
}

std::string RegisterValueBreakpoint::typeName() const noexcept
{
    return "Register pair value"s;
}

std::string RegisterValueBreakpoint::conditionDescription() const
{
    std::ostringstream out;
    out << to_string(watchedRegister()) << " == 0x" << std::hex << std::setfill('0') << std::setw(4) << targetValue();
    return out.str();
}
