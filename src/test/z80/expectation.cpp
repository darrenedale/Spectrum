//
// Created by darren on 26/02/2021.
//

#include <cstring>
#include <sstream>

#include "expectation.h"

using namespace Test::Z80;

Expectation::Expectation(std::string name, std::size_t tStates, Events events, State expectedState)
: m_name(std::move(name)),
  m_tStates(tStates),
  m_events(std::move(events)),
  m_expectedState(std::move(expectedState))
{
}

Expectation::Expectation(const Expectation & other) = default;
Expectation::Expectation(Expectation && other) noexcept = default;
Expectation & Expectation::operator=(const Expectation & other) = default;
Expectation & Expectation::operator=(Expectation && other) noexcept = default;
Expectation::~Expectation() = default;

Expectation::Failures Expectation::checkRegisterPairs(::Z80::Z80 & cpu) const
{
    Failures ret;
    auto & registers = cpu.registers();

    if (registers.af != m_expectedState.af) {
        ret.push_back({FailureType::AfIncorrect, m_expectedState.af, registers.af, {}});
    }

    if (registers.bc != m_expectedState.bc) {
        ret.push_back({FailureType::BcIncorrect, m_expectedState.bc, registers.bc, {}});
    }

    if (registers.de != m_expectedState.de) {
        ret.push_back({FailureType::DeIncorrect, m_expectedState.de, registers.de, {}});
    }

    if (registers.hl != m_expectedState.hl) {
        ret.push_back({FailureType::HlIncorrect, m_expectedState.hl, registers.hl, {}});
    }

    if (registers.ix != m_expectedState.ix) {
        ret.push_back({FailureType::IxIncorrect, m_expectedState.ix, registers.ix, {}});
    }

    if (registers.iy != m_expectedState.iy) {
        ret.push_back({FailureType::IyIncorrect, m_expectedState.iy, registers.iy, {}});
    }

    return ret;
}

Expectation::Failures Expectation::checkShadowRegisterPairs(::Z80::Z80 & cpu) const
{
    Failures ret;
    auto & registers = cpu.registers();

    if (registers.afShadow != m_expectedState.afShadow) {
        ret.push_back({FailureType::AfShadowIncorrect, m_expectedState.afShadow, registers.afShadow, {}});
    }

    if (registers.bcShadow != m_expectedState.bcShadow) {
        ret.push_back({FailureType::BcShadowIncorrect, m_expectedState.bcShadow, registers.bcShadow, {}});
    }

    if (registers.deShadow != m_expectedState.deShadow) {
        ret.push_back({FailureType::DeShadowIncorrect, m_expectedState.deShadow, registers.deShadow, {}});
    }

    if (registers.hlShadow != m_expectedState.hlShadow) {
        ret.push_back({FailureType::HlShadowIncorrect, m_expectedState.hlShadow, registers.hlShadow, {}});
    }

    return ret;
}

Expectation::Failures Expectation::checkRegisters(::Z80::Z80 & cpu) const
{
    Failures ret;
    auto & registers = cpu.registers();

    if (registers.r != m_expectedState.r) {
        ret.push_back({FailureType::RIncorrect, m_expectedState.r, registers.r, {}});
    }

    if (registers.i != m_expectedState.i) {
        ret.push_back({FailureType::IIncorrect, m_expectedState.i, registers.i, {}});
    }

    return ret;
}

Expectation::Failures Expectation::checkInterruptFlipFlops(::Z80::Z80 & cpu) const
{
    Failures ret;

    if (cpu.iff1() != m_expectedState.iff1) {
        ret.push_back({FailureType::Iff1Incorrect, m_expectedState.iff1, cpu.iff1(), {}});
    }

    if (cpu.iff2() != m_expectedState.iff2) {
        ret.push_back({FailureType::Iff2Incorrect, m_expectedState.iff2, cpu.iff2(), {}});
    }

    return ret;
}

Expectation::Failures Expectation::checkInterruptMode(::Z80::Z80 & cpu) const
{
    Failures ret;

    if (cpu.interruptMode() != m_expectedState.im) {
        ret.push_back({FailureType::InterruptModeIncorrect, m_expectedState.im, cpu.interruptMode(), {}});
    }

    return ret;
}

Expectation::Failures Expectation::checkMemory(::Z80::Z80 & cpu) const
{
    Failures ret;
    auto * memory = cpu.memory();
    auto blockIdx = 1;

    for (const auto & block : m_expectedState.memory) {
        for (auto idx = 0; idx < block.data.size(); ++idx) {
            if (memory[block.address + idx] != block.data[idx]) {
                std::string msg;

                {
                    std::ostringstream out(msg);
                    out << "Expected byte 0x" << std::hex << static_cast<int>(block.data[idx])
                        << " at 0x" << static_cast<int>(block.address + idx)
                        << " (" << static_cast<int>(block.address) << " + 0x" << idx << ")"
                        << "; found byte 0x" << static_cast<int>(memory[block.address + idx])
                        << " [defined in expectation memory block #" << blockIdx << "]";
                }

                ret.push_back({FailureType::MemoryIncorrect, block.data[idx], memory[block.address + idx], msg});
            }
        }

        ++blockIdx;
    }

    return ret;
}

Expectation::Failures Expectation::checkTStates(const size_t & tStates) const
{
    Failures ret;

    if (tStates != this->tStates()) {
        ret.push_back({FailureType::TStatesIncorrect, this->tStates(), tStates, {}});
    }

    return ret;
}
