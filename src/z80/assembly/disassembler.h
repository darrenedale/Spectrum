//
// Created by darren on 17/03/2021.
//

#ifndef Z80_ASSEMBLY_DISASSEMBLER_H
#define Z80_ASSEMBLY_DISASSEMBLER_H

#include "mnemonic.h"
#include "../types.h"

namespace Z80::Assembly
{
    class Disassembler
    {
    public:
        using Mnemonics = std::vector<Mnemonic>;

        // NOTE memorySize MUST be at least 4 - pad with 0x00 if required
        Disassembler(UnsignedByte * memory, int memorySize)
        : m_memory(memory),
          m_memorySize(memorySize),
          m_pc(0)
        {}

        Mnemonics disassembleAll() const
        {
            return disassembleFrom(0);
        }

        int address() const
        {
            return m_pc;
        }

        Mnemonics disassembleFrom(int address) const;
        Mnemonic nextMnemonic();

        bool canDisassembleMore() const
        {
            return m_pc < m_memorySize;
        }

        void reset()
        {
            m_pc = 0;
        }

        static Mnemonic disassembleOne(const Z80::UnsignedByte *);

    protected:
        static Mnemonic disassembleOnePlain(const Z80::UnsignedByte *);
        static Mnemonic disassembleOneCb(const Z80::UnsignedByte *);
        static Mnemonic disassembleOneEd(const Z80::UnsignedByte *);
        static Mnemonic disassembleOneDdOrFd(Register16, const Z80::UnsignedByte *);

    private:
        Z80::UnsignedByte * m_memory;
        int m_memorySize;
        int m_pc;
    };
}

#endif //Z80_ASSEMBLY_DISASSEMBLER_H
