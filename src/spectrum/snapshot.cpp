//
// Created by darren on 14/03/2021.
//

#include "snapshot.h"

#include <utility>
#include <cstring>

using namespace Spectrum;

Snapshot::Snapshot(Z80::UnsignedByte *memory, int memorySize)
: Snapshot({}, memory, memorySize)
{
}

Snapshot::Snapshot(Registers registers, Z80::UnsignedByte * memory, int memorySize)
: m_registers(std::move(registers)),
  m_memory{nullptr, memorySize},
  iff1(false),
  iff2(false),
  im(Z80::Z80::InterruptMode::IM0)
{
    if (memorySize) {
        m_memory.image = new Z80::UnsignedByte[memorySize];
        std::memcpy(m_memory.image, memory, memorySize);
    }
}

Snapshot::Snapshot(Snapshot::Memory memory)
: Snapshot({}, memory.image, memory.size)
{
}

Snapshot::Snapshot(Registers registers, Snapshot::Memory memory)
: Snapshot(std::move(registers), memory.image, memory.size)
{
}

Snapshot::~Snapshot() noexcept
{
    delete m_memory.image;
}

Snapshot::Snapshot(const Snapshot & other)
: Snapshot(other.m_registers, other.m_memory.image, other.m_memory.size)
{
    iff1 = other.iff1;
    iff2 = other.iff2;
    im = other.im;
}

Snapshot::Snapshot(Snapshot && other) noexcept
: m_registers(std::move(other.m_registers)),
  m_memory(other.m_memory),
  iff1(other.iff1),
  iff2(other.iff2),
  im(other.im)

{
    other.m_memory.image = nullptr;
    other.m_memory.size = 0;
}

void Snapshot::applyTo(Spectrum & spectrum) const
{
    // TODO throw if spectrum has insufficient memopry
    // TODO throw if snapshot has no memory
    auto * cpu = spectrum.z80();
    cpu->registers() = m_registers;
    cpu->setIff1(iff1);
    cpu->setIff2(iff2);
    cpu->setInterruptMode(im);
    std::memcpy(spectrum.memory(), m_memory.image, m_memory.size);
}

void Snapshot::readFrom(Spectrum & spectrum)
{
    auto * cpu = spectrum.z80();
    m_registers = cpu->registers();
    iff1 = cpu->iff1();
    iff2 = cpu->iff2();
    im = cpu->interruptMode();
    auto size = spectrum.memorySize();

    if (0 == size) {
        delete[] m_memory.image;
        m_memory.image = nullptr;
    } else {
        if (size > m_memory.size) {
            delete[] m_memory.image;
            m_memory.image = new Z80::UnsignedByte[size];
        }

        std::memcpy(m_memory.image, spectrum.memory(), size);
    }

    m_memory.size = size;
}
