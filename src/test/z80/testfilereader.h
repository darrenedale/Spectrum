//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_TESTFILEREADER_H
#define SPECTRUM_TEST_Z80_TESTFILEREADER_H

#include <optional>

#include "filereader.h"
#include "test.h"

namespace Test::Z80
{
    /**
     * See https://github.com/lkesteloot/z80-test/blob/master/z80-tests/tests.in for what a test file looks like
     */
    class TestFileReader : public FileReader
    {
    public:
        explicit TestFileReader(std::string fileName);
        std::optional<Test> nextTest();

    private:
        bool readEndOfTestMarker();
    };
}

#endif //SPECTRUM_TEST_Z80_TESTFILEREADER_H
