//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_EXPECTATION_H
#define SPECTRUM_TEST_Z80_EXPECTATION_H

#include <optional>
#include <vector>
#include <string>

#include "../../z80/z80.h"
#include "memory.h"
#include "memoryblock.h"
#include "state.h"
#include "event.h"

namespace Test::Z80
{
    using UnsignedWord = ::Z80::UnsignedWord;
    using UnsignedByte = ::Z80::UnsignedByte;

    enum class ExpectationFailure
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
    };

    class Expectation
    {
    public:
        using Failures = std::vector<ExpectationFailure>;

        Expectation(std::string description, std::size_t tStates, Events events, State expectedState);
        Expectation(const Expectation & other);
        Expectation(Expectation && other) noexcept;
        Expectation & operator=(const Expectation & other);
        Expectation & operator=(Expectation && other) noexcept;
        virtual ~Expectation();

        inline const std::string & description() const
        {
            return m_description;
        }

        inline std::string description()
        {
            return m_description;
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

    private:
        std::string m_description;
        Events m_events;
        State m_expectedState;
        std::size_t m_tStates;
    };
}

#endif //SPECTRUM_TEST_Z80_EXPECTATION_H
