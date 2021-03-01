//
// Created by darren on 26/02/2021.
//

#include <iostream>

#include "runner.h"

using namespace Test::Z80;

enum ExitCode: int
{
    Ok = 0,
    InvalidArguments,
    FailedOpeningTestFile,
};

void usage(char * exe)
{
    std::cout << "Usage: " << exe << " <test-file>\n";
}

int main(int argc, char ** argv)
{
    if (1 == argc) {
        usage(argv[0]);
        return ExitCode::InvalidArguments;
    }

    Runner testRunner(argv[1]);

    if (3 <= argc) {
        std::vector<std::string> testNames;
        int argIdx = 2;

        while (argIdx < argc) {
            testNames.emplace_back(argv[argIdx]);
            ++argIdx;
        }

        testRunner.runTests(testNames);
    } else {
        testRunner.runAllTests();
    }

    return ExitCode::Ok;
}
