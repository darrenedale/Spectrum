//
// Created by darren on 26/02/2021.
//

#include <cstring>
#include <cstdio>
#include <cctype>

#include "test.h"

using namespace Spectrum::Test::Z80;

Test::Test(std::string description, std::optional<InitialState> initialState)
: m_description(std::move(description))
{
    if (initialState) {
        m_initialState = std::move(*initialState);
    }
}

Test::Test(const Test & other) = default;
Test::Test(Test && other) noexcept = default;
Test & Test::operator=(const Test & other) = default;
Test & Test::operator=(Test && other) noexcept = default;
Test::~Test() = default;

void Test::setupZ80(::Z80 & cpu)
{
    cpu.reset();
    cpu.setAf(m_initialState.af);
    cpu.setBc(m_initialState.bc);
    cpu.setDe(m_initialState.de);
    cpu.setHl(m_initialState.hl);
    cpu.setAfShadow(m_initialState.afShadow);
    cpu.setBcShadow(m_initialState.bcShadow);
    cpu.setDeShadow(m_initialState.deShadow);
    cpu.setHlShadow(m_initialState.hlShadow);
    cpu.setIx(m_initialState.ix);
    cpu.setIy(m_initialState.iy);
    cpu.setSp(m_initialState.sp);
    cpu.setPc(m_initialState.pc);

    cpu.setI(m_initialState.i);
    cpu.setR(m_initialState.r);

    cpu.setIff1(m_initialState.iff1);
    cpu.setIff2(m_initialState.iff2);
}

void Test::setupMemory(UnsignedByte * memory)
{
    for (const auto & block : m_initialState.memory) {
        std::memcpy(memory + block.address, block.data.data(), block.data.size());
    }
}
