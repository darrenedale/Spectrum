//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_RUNNER_H
#define SPECTRUM_TEST_Z80_RUNNER_H

#include <string>
#include <optional>

#include "testbattery.h"

namespace Test::Z80
{
    class Runner
    {
    public:
        explicit Runner(std::string testFileBaseName);
        Runner(std::string testFileBaseName, ::Z80::Z80 & cpu);
        Runner(std::string testFileBaseName, ::Z80::Z80 * cpu);
        Runner(const Runner & other) = delete;
        Runner(Runner && other) = delete;
        void operator=(const Runner & other) = delete;
        void operator=(Runner && other) = delete;
        virtual ~Runner();

        const ::Z80::Z80 & cpu() const
        {
            return *m_cpu;
        }

        ::Z80::Z80 & cpu()
        {
            return *m_cpu;
        }

        bool runTest(const std::string & description);
        void runNextTest();
        void runAllTests();

    protected:
        void runTest(const TestBattery::TestCase & testCase);
        void outputZ80State(std::ostream & stream = std::cout) const;

    private:
        ::Z80::Z80 * m_cpu;
        bool m_borrowedCpu;
        bool m_testsRead;
        TestBattery m_tests;
    };
}

#endif //SPECTRUM_TEST_Z80_RUNNER_H
