//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_TESTFILEREADER_H
#define SPECTRUM_TEST_Z80_TESTFILEREADER_H

#include <cstdio>
#include <optional>

#include "test.h"

namespace Spectrum::Test::Z80
{
    /**
     * See https://github.com/lkesteloot/z80-test/blob/master/z80-tests/tests.in for what a test file looks like
     */
    class TestFileReader
    {
    public:
        explicit TestFileReader(std::FILE *);
        explicit TestFileReader(std::string fileName);
        virtual ~TestFileReader();

        inline const std::string & testFileName()
        {
            return m_inFileName;
        }

        bool isOpen() const
        {
            return static_cast<bool>(m_inFile);
        }

        bool open();

        std::optional<Test> nextTest() const;

    private:
        void skipEmptyLines() const;
        std::optional<std::string> readDescription() const;
        bool readRegisters(InitialState &) const;
        bool readFlags(InitialState &) const;
        std::optional<std::vector<MemoryBlock>> readMemoryBlocks() const;
        std::optional<MemoryBlock> readMemoryBlock() const;
        std::optional<::Z80::UnsignedWord> readWord() const;
        std::optional<::Z80::UnsignedByte> readByte() const;
        std::optional<bool> readFlag() const;
        bool m_borrowedFile;
        FILE * m_inFile;
        std::string m_inFileName;
    };
}

#endif //SPECTRUM_TEST_Z80_TESTFILEREADER_H
