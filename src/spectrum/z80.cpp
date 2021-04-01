//
// Created by darren on 11/03/2021.
//

#include "z80.h"

using namespace Spectrum;

using BaseZ80 = ::Z80::Z80;

Spectrum::Z80::Z80(MemoryType * memory)
: BaseZ80(memory)
{}

::Z80::InstructionCost Spectrum::Z80::execute(const UnsignedByte *instruction, bool doPc)
{
    auto cost = BaseZ80::execute(instruction, doPc);
    notifyObservers(m_instructionObservers);
    return cost;
}

int Spectrum::Z80::handleInterrupt()
{
    notifyObservers(m_interruptObservers);
    return BaseZ80::handleInterrupt();
}

void Spectrum::Z80::handleNmi()
{
    notifyObservers(m_nmiObservers);
    BaseZ80::handleNmi();
}
