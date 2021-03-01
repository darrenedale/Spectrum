//
// Created by darren on 26/02/2021.
//

#include <cstring>

#include "expectation.h"

using namespace Test::Z80;

Expectation::Expectation(std::string description, std::size_t tStates, Events events, State expectedState)
: m_description(std::move(description)),
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

std::vector<ExpectationFailure> Expectation::checkRegisterPairs(::Z80::Z80 & cpu) const
{
    std::vector<ExpectationFailure> ret;

    if (cpu.afRegisterValue() != m_expectedState.af) {
        ret.push_back(ExpectationFailure::AfIncorrect);
    }

    if (cpu.bcRegisterValue() != m_expectedState.bc) {
        ret.push_back(ExpectationFailure::BcIncorrect);
    }

    if (cpu.deRegisterValue() != m_expectedState.de) {
        ret.push_back(ExpectationFailure::DeIncorrect);
    }

    if (cpu.hlRegisterValue() != m_expectedState.hl) {
        ret.push_back(ExpectationFailure::HlIncorrect);
    }

    if (cpu.ixRegisterValue() != m_expectedState.ix) {
        ret.push_back(ExpectationFailure::IxIncorrect);
    }

    if (cpu.iyRegisterValue() != m_expectedState.iy) {
        ret.push_back(ExpectationFailure::IyIncorrect);
    }

    return ret;
}

std::vector<ExpectationFailure> Expectation::checkShadowRegisterPairs(::Z80::Z80 & cpu) const
{
    std::vector<ExpectationFailure> ret;

    if (cpu.afShadowRegisterValue() != m_expectedState.afShadow) {
        ret.push_back(ExpectationFailure::AfShadowIncorrect);
    }

    if (cpu.bcShadowRegisterValue() != m_expectedState.bcShadow) {
        ret.push_back(ExpectationFailure::BcShadowIncorrect);
    }

    if (cpu.deShadowRegisterValue() != m_expectedState.deShadow) {
        ret.push_back(ExpectationFailure::DeShadowIncorrect);
    }

    if (cpu.hlShadowRegisterValue() != m_expectedState.hlShadow) {
        ret.push_back(ExpectationFailure::HlShadowIncorrect);
    }

    return ret;
}

std::vector<ExpectationFailure> Expectation::checkRegisters(::Z80::Z80 & cpu) const
{
    std::vector<ExpectationFailure> ret;

    if (cpu.rRegisterValue() != m_expectedState.r) {
        ret.push_back(ExpectationFailure::RIncorrect);
    }

    if (cpu.iRegisterValue() != m_expectedState.i) {
        ret.push_back(ExpectationFailure::IIncorrect);
    }

    return ret;
}

std::vector<ExpectationFailure> Expectation::checkInterruptFlipFlops(::Z80::Z80 & cpu) const
{
    std::vector<ExpectationFailure> ret;

    if (cpu.iff1() != m_expectedState.iff1) {
        ret.push_back(ExpectationFailure::Iff1Incorrect);
    }

    if (cpu.iff2() != m_expectedState.iff2) {
        ret.push_back(ExpectationFailure::Iff2Incorrect);
    }

    return ret;
}

std::vector<ExpectationFailure> Expectation::checkInterruptMode(::Z80::Z80 & cpu) const
{
    std::vector<ExpectationFailure> ret;

    if (cpu.interruptMode() != m_expectedState.im) {
        ret.push_back(ExpectationFailure::InterruptModeIncorrect);
    }

    return ret;
}


std::vector<ExpectationFailure> Expectation::checkMemory(::Z80::Z80 & cpu) const
{
    std::vector<ExpectationFailure> ret;
    auto * memory = cpu.memory();
    bool fail = false;

    for (const auto & block : m_expectedState.memory) {
        for (auto idx = 0; idx < block.data.size(); ++idx) {
            if (memory[block.address + idx] != block.data[idx]) {
                fail = true;
                break;
            }
        }

        if (fail) {
            break;
        }
    }

    if (fail) {
        ret.push_back(ExpectationFailure::MemoryIncorrect);
    }

    return ret;
}

