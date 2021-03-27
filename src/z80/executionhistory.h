//
// Created by darren on 26/03/2021.
//

#ifndef Z80_EXECUTIONHISTORY_H
#define Z80_EXECUTIONHISTORY_H

#include <array>
#include "types.h"
#include "registers.h"
#include "assembly/operand.h"
#include "assembly/mnemonic.h"

namespace Z80
{
    struct ExecutedInstruction
    {
        ExecutedInstruction()
        : machineCode{0x00, 0x00, 0x00, 0x00},
          mnemonic(),
          operandValues(),
          registersBefore(),
          registersAfter(),
          stack({.top = nullptr, .size = 0})
        {}

        virtual ~ExecutedInstruction()
        {
            delete[] stack.top;
            stack.top = nullptr;
            stack.size = 0;
        }

        // TODO date/time/tstates?
        UnsignedByte machineCode[4];
        Assembly::Mnemonic mnemonic;
        std::vector<Assembly::OperandValue> operandValues;
        Registers registersBefore;
        Registers registersAfter;

        struct {
            UnsignedByte * top;
            UnsignedWord size;
        } stack;
    };

    template <std::size_t size>
    using ExecutionHistoryBase = std::array<ExecutedInstruction, size>;

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

        const_iterator oldest() const
        {
            return m_next;
        }

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
