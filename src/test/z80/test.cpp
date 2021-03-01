//
// Created by darren on 26/02/2021.
//

#include <cstring>

#include "test.h"

using TestClass = Test::Z80::Test;

TestClass::Test(std::string name, std::size_t tStates, std::optional<State> initialState)
: m_name(std::move(name)),
  m_tStates(tStates)
{
    if (initialState) {
        m_initialState = std::move(*initialState);
    }
}

TestClass::Test(const TestClass & other) = default;
TestClass::Test(TestClass && other) noexcept = default;
TestClass & TestClass::operator=(const TestClass & other) = default;
TestClass & TestClass::operator=(TestClass && other) noexcept = default;
TestClass::~Test() = default;

void TestClass::setupZ80(::Z80::Z80 & cpu) const
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

void TestClass::setupMemory(UnsignedByte * memory) const
{
    for (const auto & block : m_initialState.memory) {
        std::memcpy(memory + block.address, block.data.data(), block.data.size());
    }
}
