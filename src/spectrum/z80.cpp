//
// Created by darren on 11/03/2021.
//

#include "z80.h"

using namespace Spectrum;

Spectrum::Z80::Z80(MemoryType * memory)
: CoreZ80Cpu(memory)
{}

CoreZ80::InstructionCost Spectrum::Z80::execute(const UnsignedByte *instruction, const bool doPc)
{
    const auto cost = CoreZ80Cpu::execute(instruction, doPc);
    notifyObservers(m_instructionObservers);
    return cost;
}

int Spectrum::Z80::handleInterrupt()
{
    notifyObservers(m_interruptObservers);
    return CoreZ80Cpu::handleInterrupt();
}

void Spectrum::Z80::handleNmi()
{
    notifyObservers(m_nmiObservers);
    CoreZ80Cpu::handleNmi();
}
