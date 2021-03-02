//
// Created by darren on 27/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_TESTBATTERY_H
#define SPECTRUM_TEST_Z80_TESTBATTERY_H

#include <string>
#include <unordered_map>
#include <optional>

#include "test.h"
#include "expectation.h"

namespace Test::Z80
{
    class TestBattery
    {
    public:
        struct TestCase
        {
            Test test;
            std::optional<Expectation> expectation;
        };

        using TestCases = std::unordered_map<std::string, TestCase>;

        explicit TestBattery(std::string baseFileName);

        bool readTests();

        int count() const
        {
            return static_cast<int>(m_tests.size());
        }

        const TestCases & testCases() const
        {
            return m_tests;
        }

        void reset() const
        {
            m_currentTestCase.reset();
        }

        bool hasMoreTestCases() const
        {
            if (!m_currentTestCase) {
                return 0 < count();
            }

            return *m_currentTestCase != m_tests.cend();
        }

        const TestCase * nextTestCase() const
        {
            if (0 == count()) {
                return nullptr;
            }

            if (!m_currentTestCase) {
                m_currentTestCase = m_tests.cbegin();
            } else {
                (*m_currentTestCase)++;
            }

            if (*m_currentTestCase == m_tests.cend()) {
                return nullptr;
            }

            return &((*m_currentTestCase)->second);
        }

    private:
        std::string m_baseFileName;
        TestCases m_tests;
        mutable std::optional<TestCases::const_iterator> m_currentTestCase;
    };
}

#endif //SPECTRUM_TEST_Z80_TESTBATTERY_H
