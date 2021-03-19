//
// Created by darren on 14/03/2021.
//

#include <cstring>

#include "snapshot.h"
#include "displaydevice.h"

using namespace Spectrum;

using InterruptMode = ::Z80::InterruptMode;

Snapshot::Snapshot(const Spectrum & spectrum)
: Snapshot(spectrum.z80()->registers(), spectrum.memory(), spectrum.memorySize())
{
    auto * cpu = spectrum.z80();
    iff1 = cpu->iff1();
    iff2 = cpu->iff2();
    im = cpu->interruptMode();

    if (!spectrum.displayDevices().empty()) {
        border = spectrum.displayDevices()[0]->border();
    }
}

Snapshot::Snapshot(Z80::UnsignedByte *memory, int memorySize)
: Snapshot({}, memory, memorySize)
{
}

Snapshot::Snapshot(const ::Z80::Registers & registers, Z80::UnsignedByte * memory, int memorySize)
: m_registers(),
  m_memory{nullptr, memorySize},
  iff1(false),
  iff2(false),
  im(InterruptMode::IM0),
  border(Colour::White)
{
    copyRegisters(registers);

    if (memorySize) {
        m_memory.image = new Z80::UnsignedByte[memorySize];
        std::memcpy(m_memory.image, memory, memorySize);
    }
}

Snapshot::~Snapshot() noexcept
{
    delete m_memory.image;
}

Snapshot::Snapshot(const Snapshot & other)
: Snapshot({}, other.m_memory.image, other.m_memory.size)
{
    m_registers = other.m_registers;
    iff1 = other.iff1;
    iff2 = other.iff2;
    im = other.im;
    border = other.border;
}

Snapshot::Snapshot(Snapshot && other) noexcept
: m_registers(other.m_registers),
  m_memory(other.m_memory),
  iff1(other.iff1),
  iff2(other.iff2),
  im(other.im),
  border(other.border)
{
    other.m_memory.image = nullptr;
    other.m_memory.size = 0;
}

void Snapshot::applyTo(Spectrum & spectrum) const
{
    // TODO throw if spectrum has insufficient memory
    // TODO throw if snapshot has no memory
    auto * cpu = spectrum.z80();
    auto & registers = cpu->registers();
    registers.af = m_registers.af;
    registers.bc = m_registers.bc;
    registers.de = m_registers.de;
    registers.hl = m_registers.hl;
    registers.ix = m_registers.ix;
    registers.iy = m_registers.iy;

    registers.afShadow = m_registers.afShadow;
    registers.bcShadow = m_registers.bcShadow;
    registers.deShadow = m_registers.deShadow;
    registers.hlShadow = m_registers.hlShadow;

    registers.memptr = m_registers.memptr;

    registers.i = m_registers.i;
    registers.r = m_registers.r;
    
    cpu->setIff1(iff1);
    cpu->setIff2(iff2);
    cpu->setInterruptMode(im);
    std::memcpy(spectrum.memory(), m_memory.image, m_memory.size);
}

void Snapshot::readFrom(Spectrum & spectrum)
{
    auto * cpu = spectrum.z80();
    copyRegisters(cpu->registers());
    iff1 = cpu->iff1();
    iff2 = cpu->iff2();
    im = cpu->interruptMode();

    if (!spectrum.displayDevices().empty()) {
        border = spectrum.displayDevices()[0]->border();
    }

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

void Snapshot::copyRegisters(const ::Z80::Registers & registers)
{
    m_registers.af = registers.af;
    m_registers.bc = registers.bc;
    m_registers.de = registers.de;
    m_registers.hl = registers.hl;
    m_registers.ix = registers.ix;
    m_registers.iy = registers.iy;
    m_registers.pc = registers.pc;
    m_registers.sp = registers.sp;

    m_registers.afShadow = registers.afShadow;
    m_registers.bcShadow = registers.bcShadow;
    m_registers.deShadow = registers.deShadow;
    m_registers.hlShadow = registers.hlShadow;

    m_registers.i = registers.i;
    m_registers.r = registers.r;

    m_registers.a = registers.a;
    m_registers.f = registers.f;
    m_registers.b = registers.b;
    m_registers.c = registers.c;
    m_registers.d = registers.d;
    m_registers.e = registers.e;
    m_registers.h = registers.h;
    m_registers.l = registers.l;

    m_registers.aShadow = registers.aShadow;
    m_registers.fShadow = registers.fShadow;
    m_registers.bShadow = registers.bShadow;
    m_registers.cShadow = registers.cShadow;
    m_registers.dShadow = registers.dShadow;
    m_registers.eShadow = registers.eShadow;
    m_registers.hShadow = registers.hShadow;
    m_registers.lShadow = registers.lShadow;

    m_registers.memptr = registers.memptr;
}

Snapshot & Snapshot::operator=(const Snapshot & other)
{
    m_registers = other.m_registers;
    iff1 = other.iff1;
    iff2 = other.iff2;
    im = other.im;
    border = other.border;

    if (0 == other.m_memory.size) {
        delete[] m_memory.image;
    } else {
        if (m_memory.size < other.m_memory.size) {
            delete[] m_memory.image;
            m_memory.image = new Z80::UnsignedByte[other.m_memory.size];
        }

        std::memcpy(m_memory.image, other.m_memory.image, other.m_memory.size);
    }

    m_memory.size = other.m_memory.size;
    return *this;
}

Snapshot & Snapshot::operator=(Snapshot && other) noexcept
{
    m_registers = other.m_registers;
    iff1 = other.iff1;
    iff2 = other.iff2;
    im = other.im;
    border = other.border;
    m_memory = other.m_memory;
    other.m_memory.image = nullptr;
    other.m_memory.size = 0;
    return *this;
}
