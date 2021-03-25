//
// Created by darren on 17/03/2021.
//

#ifndef Z80_ASSEMBLY_DISASSEMBLER_H
#define Z80_ASSEMBLY_DISASSEMBLER_H

#include "mnemonic.h"
#include "../types.h"
#include "../z80.h"

namespace Z80::Assembly
{
    /**
     * A Z80 machine code disassembler.
     *
     * TODO disassembly of 0xdd 0xcb and 0xfd 0xcb extended opcodes has not yet been implemented.
     *
     * The disassembler can work in a number of ways:
     * - You can use it statically by providing machine code fragments to the static method disassembleOne()
     * - You can use it to stream instructions sequentially from machine code in a memory image
     * - You can use it to disassemble batches of instructions from specified addresses inside a memory image
     *
     * ## Disassembling statically
     * This is the simplest usage scenario - Call Disassembler::disassembleOne() with an array of at least four bytes
     * and it will return a Mnemonic representing the machine code instruction at the first byte. (If you know for sure
     * that an array of fewer than four bytes is a complete, valid Z80 machine code instruction, you can use that - four
     * bytes is the best way to guarantee that you'll receive a mnemonic without overflowing the array. In other words,
     * the caller is responsible for ensuring the program doesn't segfault.)
     *
     * ## Streaming disassembly
     * Create a new instance with a memory image, and call nextMnemonic() in a loop guarded by a call to
     * canDisassembleMore(). This will disassemble the entire memory image one instruction at a time:
     *
     *     while (disassembler.canDisassembleMore()) {
     *         std::cout << disassembler.nextMnemonic() << '\n';
     *     }
     *
     * ## Batch disassembly
     * This gives you most control over the disassembly. Create a new instance with a memmory image, and call
     * disassembleFrom() with the address of the first machine code instruction and (optionally) the number of
     * instructions you want to disassemble at most. This will return a vector of instructions up to the requested size
     * in length. It is the caller's responsibility to keep track of where in memory the disassembly has reached if
     * that's important to it.
     *
     * This way of using the disassembler can be useful when, for example, you've got an emulator paused at a breakpoint
     * and you want to display a disassembly from the Z80's current program counter.
     */
    class Disassembler
    {
    public:
        using Mnemonics = std::vector<Mnemonic>;

        /**
         * Initialise a new disassembler instance.
         *
         * The provided memory must be at least 4 bytes in size, and it must be guaranteed to be a valid pointer to a
         * block of at least memorySize bytes for the lifetime of the disassembler. The disassembler does not take
         * ownership of the memory: the owner of the memory must ensure that the disassembler is discarded once the
         * memory is no longer valid.
         *
         * @param memory
         * @param memorySize
         */
        explicit Disassembler(::Z80::Z80::MemoryType * memory)
        : m_memory(memory),
          m_pc(0)
        {}

        /**
         * Attempt to disassemble the entire memory from start to finish.
         *
         * @return
         */
        Mnemonics disassembleAll() const
        {
            return disassembleFrom(0);
        }

        /**
         * Fetch the current internal address pointer.
         *
         * The address pointer is used with the methods that treat the memory as a stream of Z80 machine code:
         * canDisassembleMore() and nextMnemonic().
         *
         * @return
         */
        int address() const
        {
            return m_pc;
        }

        /**
         * Disassemble mnemonics from a given address, optionally with a maximum number of mnemonics to return.
         *
         * If no count or a negative count is provided, the disassembler will attempt to disassemble all the mnemonics
         * from address to the end of the memory.
         *
         * The disassembled mnemonics will be returned when the requested number have been disassembled or the memory
         * has been exhausted.
         *
         * @param address
         * @param maxCount
         * @return
         */
        Mnemonics disassembleFrom(int address, int maxCount = -1) const;

        /**
         * Disassemble the next mnemonic.
         *
         * The mnemonic at the internal address pointer is disassembled and returned. Be sure to check
         * canDisassembleMore() first to ensure that the internal address pointer is within the memory. If it is outside
         * the memory, you will continue to receive a stream of NOP instructions indefinitely if you don't check first.
         *
         * @return
         */
        Mnemonic nextMnemonic();

        /**
         * Check whether there are more machine code instructions in the memory to disassemble.
         *
         * Once the internal memory address pointer has passed the end of the memory this returns false. You can call
         * reset() to set the internal memory address pointer back to the beginning of the memory.
         *
         * @return
         */
        bool canDisassembleMore() const
        {
            return m_pc < m_memory->size();
        }

        /**
         * Set the internal memory address to 0x0000. The next call to nextMnemonic() will attempt to disassemble the
         * instruction at the beginning of the memory.
         */
        void reset()
        {
            m_pc = 0;
        }

        /**
         * Disassemble a single instruction from a provided machine code fragment.
         *
         * The fragment must be guaranteed to be large enough to represent at least one valid instruction. The easiest
         * way to achieve this is to make sure that the fragment is at least four bytes in length.
         *
         * You can use this if you have some bytes that represent Z80 machine code and you want to quickly disassemble
         * them. Its primary purpose, however, is as an internal helper to do the main work of disassembling the machine
         * code in the instance's memory.
         *
         * @return
         */
        static Mnemonic disassembleOne(const ::Z80::UnsignedByte *);

    protected:
        /**
         * Helper to disassemble a plain opcode from a machine code fragment.
         *
         * The fragment must be guaranteed to be large enough to represent at least one valid instruction. The easiest
         * way to achieve this is to make sure that the fragment is at least four bytes in length.
         *
         * @return
         */
        static Mnemonic disassembleOnePlain(const ::Z80::UnsignedByte *);

        /**
         * Helper to disassemble an 0xcb extended opcode from a machine code fragment.
         *
         * The fragment must start with the byte immediately following the 0xcb prefix.
         *
         * The fragment is provided as a pointer for consistency in the interface, but in reality all 0xcb extended
         * opcodes are two bytes in length, meaning that only the first byte in the provided fragment will be accessed.
         *
         * @return
         */
        static Mnemonic disassembleOneCb(const ::Z80::UnsignedByte *);

        /**
         * Helper to disassemble an 0xed extended opcode from a machine code fragment.
         *
         * The fragment must start with the byte immediately following the 0xed prefix and must be guaranteed to be
         * large enough to represent at least one valid 0xed extended instruction. The easiest way to achieve this is to
         * make sure that the fragment is at least three bytes in length.
         *
         * @return
         */
        static Mnemonic disassembleOneEd(const ::Z80::UnsignedByte *);

        /**
         * Helper to disassemble an 0xdd or 0xfd extended opcode from a machine code fragment.
         *
         * Instructions prefixed with 0xdd and 0xfs deal with the index registers. 0xdd signifies instructions that work
         * with IX and 0xfd with IY. When calling, the register provided must be one of these two registers.
         *
         * The fragment must start with the byte immediately following the 0xdd or 0xfd prefix and must be guaranteed to
         * be large enough to represent at least one valid 0xdd or 0xfd extended instruction. The easiest way to achieve
         * this is to make sure that the fragment is at least three bytes in length.
         *
         * @return
         */
        static Mnemonic disassembleOneDdOrFd(Register16, const ::Z80::UnsignedByte *);

        static Mnemonic disassembleOneDdCbOrFdCb(Register16, const ::Z80::UnsignedByte *);

    private:
        // the memory image the disassembler will work with
        ::Z80::Z80::MemoryType * m_memory;

        // the internal address pointer for the disassembly streaming API
        int m_pc;
    };
}

#endif //Z80_ASSEMBLY_DISASSEMBLER_H
