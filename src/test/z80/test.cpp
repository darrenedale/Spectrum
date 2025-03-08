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
    auto & registers = cpu.registers();

    registers.af = m_initialState.af;
    registers.bc = m_initialState.bc;
    registers.de = m_initialState.de;
    registers.hl = m_initialState.hl;
    registers.afShadow = m_initialState.afShadow;
    registers.bcShadow = m_initialState.bcShadow;
    registers.deShadow = m_initialState.deShadow;
    registers.hlShadow = m_initialState.hlShadow;
    registers.ix = m_initialState.ix;
    registers.iy = m_initialState.iy;
    registers.sp = m_initialState.sp;
    registers.pc = m_initialState.pc;

    registers.memptr = m_initialState.memptr;

    registers.i = m_initialState.i;
    registers.r = m_initialState.r;

    cpu.setIff1(m_initialState.iff1);
    cpu.setIff2(m_initialState.iff2);
}

void TestClass::setupMemory(::Z80::Z80::Memory & memory) const
{
    for (const auto & block : m_initialState.memory) {
        memory.writeBytes(block.address, block.data.size(), block.data.data());
    }
}
