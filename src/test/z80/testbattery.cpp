//
// Created by darren on 27/02/2021.
//

#include <iostream>

#include "testbattery.h"
#include "testfilereader.h"
#include "expectationfilereader.h"

Test::Z80::TestBattery::TestBattery(std::string baseFileName)
: m_baseFileName(std::move(baseFileName))
{
}

bool Test::Z80::TestBattery::readTests()
{
    TestCases cases;
    auto testReader = TestFileReader(m_baseFileName + ".in");

    if (!testReader.open()) {
        return false;
    }

    std::optional<Test> test;

    while (!testReader.eof() && (test = testReader.nextTest())) {
        auto [it, insert] = cases.insert_or_assign(test->name(), TestCase({*test, {}}));

        if (insert) {
            std::cerr << "";
        }
    }

    if (!testReader.eof()) {
        return false;
    }

    auto expectationReader = ExpectationFileReader(m_baseFileName + ".expected");

    if (!expectationReader.open()) {
        return false;
    }

    std::optional<Expectation> expectation;

    while (!expectationReader.eof() && (expectation = expectationReader.nextExpectation())) {
        auto testCase = cases.find(expectation->name());
        if (cases.end() == testCase) {
            // found expectation for test that wasn't read
            continue;
        }

        testCase->second.expectation = *expectation;
    }

    if (!expectationReader.eof()) {
        return false;
    }

    m_tests = std::move(cases);
    reset();
    return true;
}
