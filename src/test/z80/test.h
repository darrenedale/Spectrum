//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_H
#define SPECTRUM_TEST_H

#include <optional>
#include <vector>
#include <string>

#include "../../z80/z80.h"
#include "memory.h"
#include "memoryblock.h"
#include "state.h"

namespace Test::Z80
{
    class Test
    {
    public:
        explicit Test(std::string description, std::size_t tStates, std::optional<State> initialState = {});
        Test(const Test & other);
        Test(Test && other) noexcept;
        Test & operator=(const Test & other);
        Test & operator=(Test && other) noexcept;
        virtual ~Test();

        inline const std::string & description() const
        {
            return m_description;
        }

        inline std::string description()
        {
            return m_description;
        }

        void setupZ80(::Z80::Z80 & cpu) const;
        void setupMemory(UnsignedByte * memory) const;

        std::size_t tStates() const
        {
            return m_tStates;
        }

    private:
        std::string m_description;
        State m_initialState;
        std::size_t m_tStates;
    };
}

#endif //SPECTRUM_TEST_H
