//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_EXPECTEDFILEREADER_H
#define SPECTRUM_TEST_Z80_EXPECTEDFILEREADER_H

#include <optional>

#include "filereader.h"
#include "expectation.h"
#include "event.h"

namespace Test::Z80
{
    /**
     * See https://github.com/lkesteloot/z80-test/blob/master/z80-tests/tests.expected for what an expected file looks
     * like
     */
    class ExpectationFileReader : public FileReader
    {
    public:
        explicit ExpectationFileReader(std::string fileName);
        std::optional<Expectation> nextExpectation();

    private:
        std::optional<std::vector<Event>> readEvents();
        std::optional<Event> readEvent();
    };
}

#endif //SPECTRUM_TEST_Z80_EXPECTEDFILEREADER_H
