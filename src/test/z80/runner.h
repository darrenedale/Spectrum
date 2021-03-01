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

        /**
         * Run a single named test.
         *
         * @param name The name of the test to run.
         * @return Whether or not the test was found and run.
         */
        bool runTest(const std::string & name);

        /**
         * @return Count of failures encountered.
         */
        int runNextTest();

        /**
         * @return Count of tests executed, count of failures encountered
         */
        std::tuple<int, int> runAllTests();

        /**
         * Run a subset of named tests.
         *
         * @param tests The names of the tests to run.
         *
         * @return The count of tests executed, the count of failures encountered.
         */
        std::tuple<int, int> runTests(const std::vector<std::string> & tests);

    protected:
        /**
         * @param testCase
         * @return Count of failures encountered.
         */
        int runTest(const TestBattery::TestCase & testCase);
        void outputZ80State(std::ostream & stream = std::cout) const;

    private:
        ::Z80::Z80 * m_cpu;
        bool m_borrowedCpu;
        bool m_testsRead;
        TestBattery m_tests;

    };
}

#endif //SPECTRUM_TEST_Z80_RUNNER_H
