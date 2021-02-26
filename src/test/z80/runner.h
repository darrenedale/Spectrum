//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_RUNNER_H
#define SPECTRUM_TEST_Z80_RUNNER_H

#include <string>
#include <optional>

#include "testfilereader.h"

namespace Spectrum::Test::Z80
{
    class Runner
    {
    public:
        explicit Runner(std::string testFile);
        Runner(std::string testFile, ::Z80 & cpu);
        Runner(std::string testFile, ::Z80 * cpu);
        Runner(const Runner & other) = delete;
        Runner(Runner && other) = delete;
        void operator=(const Runner & other) = delete;
        void operator=(Runner && other) = delete;
        virtual ~Runner();

        const std::optional<Test> & currentTest()
        {
            return this->m_currentTest;
        }

        const ::Z80 & cpu() const
        {
            return *m_cpu;
        }

        ::Z80 & cpu()
        {
            return *m_cpu;
        }

        void runNextTest();
        void runAllTests();

    private:
        ::Z80 * m_cpu;
        bool m_borrowedCpu;
        TestFileReader m_reader;
        std::optional<Test> m_currentTest;
    };
}

#endif //SPECTRUM_TEST_Z80_RUNNER_H
