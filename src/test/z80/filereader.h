//
// Created by darren on 27/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_FILEREADER_H
#define SPECTRUM_TEST_Z80_FILEREADER_H

#include <string>
#include <optional>
#include <fstream>

#include "test.h"
#include "../../z80/z80.h"

namespace Test::Z80
{
    class FileReader
    {
    public:
        explicit FileReader(std::string fileName);
        virtual ~FileReader();

        inline const std::string & fileName()
        {
            return m_inFileName;
        }

        bool isOpen() const
        {
            return m_inStream.is_open();
        }

        bool open();

        bool eof() const
        {
            return isOpen() && m_inStream.eof();
        }

    protected:
        static std::optional<::Z80::UnsignedWord> parseWord(char[4]);
        static std::optional<::Z80::UnsignedByte> parseByte(char[2]);
        static std::optional<bool> parseFlag(char);
        static std::optional<InterruptMode> parseInterruptMode(char);

        void skipEmptyLines();
        std::optional<std::string> readLine();
        std::optional<std::string> readName();
        bool readRegisterPairs(State &);
        bool readRegistersFlagsTStates(State &, std::size_t &);
        std::optional<std::vector<MemoryBlock>> readMemoryBlocks();
        std::optional<MemoryBlock> readMemoryBlock();

        std::ifstream m_inStream;
        std::string m_inFileName;
    };
}

#endif //SPECTRUM_TEST_Z80_FILEREADER_H
