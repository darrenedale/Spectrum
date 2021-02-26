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

namespace Spectrum::Test::Z80
{
    using UnsignedWord = ::Z80::UnsignedWord;
    using UnsignedByte = ::Z80::UnsignedByte;

    struct MemoryBlock
    {
        UnsignedWord address;
        std::vector<UnsignedByte> data;
    };

    struct InitialState
    {
        UnsignedWord af;
        UnsignedWord bc;
        UnsignedWord de;
        UnsignedWord hl;
        UnsignedWord afShadow;
        UnsignedWord bcShadow;
        UnsignedWord deShadow;
        UnsignedWord hlShadow;
        UnsignedWord ix;
        UnsignedWord iy;
        UnsignedWord sp;
        UnsignedWord pc;
        UnsignedWord memptr;

        UnsignedByte i;
        UnsignedByte r;
        bool iff1;
        bool iff2;
        bool im;
        bool halted;
        std::size_t tStates;

        std::vector<MemoryBlock> memory;
    };

    class Test
    {
    public:
        explicit Test(std::string description, std::optional<InitialState> initialState = {});
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

        void setupZ80(::Z80 & cpu);
        void setupMemory(UnsignedByte * memory);

    private:
        std::string m_description;
        InitialState m_initialState;
    };
}

#endif //SPECTRUM_TEST_H
