//
// Created by darren on 27/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_MEMORYBLOCK_H
#define SPECTRUM_TEST_Z80_MEMORYBLOCK_H

namespace Test::Z80
{
    using UnsignedWord = ::Z80::UnsignedWord;
    using UnsignedByte = ::Z80::UnsignedByte;

    struct MemoryBlock
    {
        UnsignedWord address;
        std::vector<UnsignedByte> data;
    };
}

#endif //SPECTRUM_TEST_Z80_MEMORYBLOCK_H
