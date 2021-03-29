//
// Created by darren on 26/03/2021.
//

#ifndef Z80_EXECUTIONHISTORY_H
#define Z80_EXECUTIONHISTORY_H

#include <array>
#include "../memory.h"
#include "types.h"
#include "registers.h"
#include "assembly/operand.h"
#include "assembly/mnemonic.h"

namespace Z80
{
    /**
     * Capture enough state when an instruction is executed to enable disassembly of that instruction at a later time.
     *
     * The state captured includes the instruction machine code, the Z80 registers and fragments of memory such that
     * all potential operands to the instruction can have their values reconstituted if the instruction is disassembled.
     *
     * The use case it to be able to capture an execution trace and rebuild the history of instructions that led to a
     * given point in order to be able to examine precisely where the emulation placed the Z80 in an incorrect state.
     *
     * This is very rough-and-ready and has not been tested. It should only be used in debug builds. I could easily have
     * missed things (e.g. possible operands that cannot be evaluated, operands being evaluated incorrectly or being
     * presented in the wrong byte order) but it has proven safe enough so far.
     */
    struct ExecutedInstruction
    {
        /**
         * Default constructor is required because the ring buffer uses a fixed-size array.
         */
        ExecutedInstruction()
        : machineCode{0x00, 0x00, 0x00, 0x00},
          registersBefore(),
          registersAfter(),
          stack({.top = nullptr, .size = 0}),
          memoryFragments()
        {}

        /**
         * Initialises a new instance from the machine code provided, extracting state from the Z80 provided.
         *
         * @param instruction
         * @param z80
         */
        ExecutedInstruction(const UnsignedByte instruction[4], ::Z80::Z80 * z80);

        virtual ~ExecutedInstruction()
        {
            delete[] stack.top;
            stack.top = nullptr;
            stack.size = 0;
        }

        /**
         * This will populate the memory fragments.
         *
         * It must ONLY be called after the machine code and before registers have been set. The memory fragments enable
         * access to the relevant memory state to enable the machine code to be disassembled long after the instruction
         * was actually executed. This means that we don't have to disassemble the instruction while the history is
         * being captured (which is costly), and can defer it until we need to examine the history. The memory fragments
         * provide just enough context that operand values can be evaluated.
         *
         * @param z80
         */
        void readMemoryFragments(::Memory<UnsignedByte> * memory);

        /**
         * Evaluate an operand based on the state stored in the executed instruction.
         *
         * @param operand The operand to evaluate.
         * @param asDestination Whether or not the operand is the source or destination operand in the instruction.
         * @return
         */
        [[nodiscard]] Assembly::OperandValue evaluateOperand(const Assembly::Operand & operand, bool asDestination) const;

        UnsignedByte machineCode[4];
        Registers registersBefore;
        Registers registersAfter;

        struct {
            UnsignedByte * top;
            UnsignedWord size;
        } stack;

        struct {
            UnsignedByte indirectPcPlus1[2];        // two bytes addressed by [PC + 1, PC + 2]
            UnsignedByte indirectPcPlus2[2];        // two bytes addressed by [PC + 1, PC + 2]
            UnsignedByte indirectHl[2];             // two bytes address by the content of HL
            UnsignedByte indirectDe[2];             // two bytes address by the content of DE
            UnsignedByte indirectBc[2];             // two bytes address by the content of BC
            UnsignedByte indirectSp[2];             // two bytes on the top of the stack

            // for these, note that d is *always* the signed byte at PC + 2
            UnsignedByte indirectIxWithOffset[2];   // two bytes address by the content of IX + d
            UnsignedByte indirectIyWithOffset[2];   // two bytes address by the content of IY + d
        } memoryFragments;
    };

    template <std::size_t size>
    using ExecutionHistoryBase = std::array<ExecutedInstruction, size>;

    /**
     * A very rough ring buffer containing a fixed-size history of instructions executed.
     *
     * @tparam size
     */
    template <std::size_t size>
    class ExecutionHistory
    : public ExecutionHistoryBase<size>
    {
    public:
        using iterator = typename std::array<ExecutedInstruction, size>::iterator;
        using const_iterator = typename std::array<ExecutedInstruction, size>::const_iterator;

        ExecutionHistory()
        : std::array<ExecutedInstruction, size>()
        {
            m_next = ExecutionHistoryBase<size>::begin();
        }

        /**
         * Add an instruction to the buffer.
         *
         * @param instruction
         */
        void add(const ExecutedInstruction & instruction)
        {
            *m_next = instruction;
            ++m_next;

            if (m_next == ExecutionHistoryBase<size>::end()) {
                m_next = ExecutionHistoryBase<size>::begin();
            }
        }

        void add(ExecutedInstruction && instruction)
        {
            *m_next = instruction;
            ++m_next;

            if (m_next == ExecutionHistoryBase<size>::end()) {
                m_next = ExecutionHistoryBase<size>::begin();
            }
        }

        /**
         * The oldest entry in the ring buffer.
         *
         * Increment the iterator to move from oldest to newest entries.
         *
         * NOTE the buffer has been exhausted not when the iterator reaches end() but when it reaches oldest() again.
         * The caller must wrap the iterator back around to begin() when it reaches end() in order to iterate the entire
         * buffer. This is a long way from ideal, but this is just a quick-and-dirty debugging mechanism. I may
         * eventually get around to writing a proper ring buffer.
         *
         * @return
         */
        const_iterator oldest() const
        {
            return m_next;
        }

        /**
         * The oldest entry in the ring buffer.
         *
         * Decrement the iterator to move from newest to oldest entries.
         *
         * NOTE the buffer has been exhausted not when the iterator reaches begin() or rend() but when it reaches
         * newest() again. The caller must wrap the iterator back around to end() - 1 when it reaches begin() in order
         * to iterate the entire buffer. This is a long way from ideal, but this is just a quick-and-dirty debugging
         * mechanism. I may eventually get around to writing a proper ring buffer.
         *
         * @return
         */
        const_iterator newest() const
        {
            if (m_next == ExecutionHistoryBase<size>::begin()) {
                return ExecutionHistoryBase<size>::end() - 1;
            }

            return m_next - 1;
        }

    private:
        iterator m_next;
    };
}

#endif // Z80_EXECUTIONHISTORY_H
