//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_EXPECTATION_H
#define SPECTRUM_TEST_Z80_EXPECTATION_H

#include <optional>
#include <vector>
#include <string>
#include <any>

#include "../../z80/z80.h"
#include "memory.h"
#include "memoryblock.h"
#include "state.h"
#include "event.h"

namespace Test::Z80
{
    using UnsignedWord = ::Z80::UnsignedWord;
    using UnsignedByte = ::Z80::UnsignedByte;

    class Expectation
    {
    public:
        enum class FailureType
        {
            AfIncorrect,
            BcIncorrect,
            DeIncorrect,
            HlIncorrect,
            IxIncorrect,
            IyIncorrect,
            AfShadowIncorrect,
            BcShadowIncorrect,
            DeShadowIncorrect,
            HlShadowIncorrect,
            IIncorrect,
            RIncorrect,
            Iff1Incorrect,
            Iff2Incorrect,
            InterruptModeIncorrect,
            MemoryIncorrect,
            TStatesIncorrect,
        };

        struct Failure
        {
            FailureType type;
            std::any expected;
            std::any actual;
            std::string message;
        };

        using Failures = std::vector<Failure>;

        Expectation(std::string name, std::size_t tStates, Events events, State expectedState);
        Expectation(const Expectation & other);
        Expectation(Expectation && other) noexcept;
        Expectation & operator=(const Expectation & other);
        Expectation & operator=(Expectation && other) noexcept;
        virtual ~Expectation();

        inline const std::string & name() const
        {
            return m_name;
        }

        std::size_t tStates() const
        {
            return m_tStates;
        }

        Failures checkRegisterPairs(::Z80::Z80 & cpu) const;
        Failures checkShadowRegisterPairs(::Z80::Z80 & cpu) const;
        Failures checkRegisters(::Z80::Z80 & cpu) const;
        Failures checkInterruptFlipFlops(::Z80::Z80 & cpu) const;
        Failures checkInterruptMode(::Z80::Z80 & cpu) const;
        Failures checkMemory(::Z80::Z80 & cpu) const;
        Failures checkTStates(const std::size_t & tStates) const;

    private:
        std::string m_name;
        Events m_events;
        State m_expectedState;
        std::size_t m_tStates;
    };
}

#endif //SPECTRUM_TEST_Z80_EXPECTATION_H
