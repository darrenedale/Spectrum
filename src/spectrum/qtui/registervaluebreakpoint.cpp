//
// Created by darren on 31/03/2021.
//

#include <sstream>
#include <iomanip>
#include "registervaluebreakpoint.h"
#include "../../z80/types.h"

using namespace Spectrum::QtUi;

bool RegisterValueBreakpoint::operator==(const Breakpoint & other) const
{
    if (typeid(*this) != typeid(other)) {
        return false;
    }

    const auto & rvOther = *reinterpret_cast<const RegisterValueBreakpoint *>(&other);
    return watchedRegister() == rvOther.watchedRegister() && targetValue() == rvOther.targetValue();
}

bool RegisterValueBreakpoint::check(const Spectrum::BaseSpectrum & spectrum)
{
    return spectrum.z80()->registerValue(watchedRegister()) == targetValue();
}

std::string RegisterValueBreakpoint::typeName() const
{
    return "Register pair value";
}

std::string RegisterValueBreakpoint::conditionDescription() const
{
    std::ostringstream out;
    out << std::to_string(watchedRegister()) << " == 0x" << std::hex << std::setfill('0') << std::setw(4) << targetValue();
    return out.str();
}