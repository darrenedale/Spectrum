//
// Created by darren on 26/02/2021.
//

#include <iostream>

#include "runner.h"
#include "testfilereader.h"
#include "test.h"

using namespace Test::Z80;

enum ExitCode: int
{
    Ok = 0,
    InvalidArguments,
    FailedOpeningTestFile,
};

struct Options
{
    bool onlyShowFails = false;
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

//    Options opts;

//    while (argIdx < argc && '-' == *argv[argIdx]) {
//        std::string arg(argv[argIdx]);
//
//        if ("--only-show-fails" == arg) {
//            opts.onlyShowFails = true;
//        }
//
//        ++argIdx;
//    }

    Runner testRunner(argv[1]);

    if (3 <= argc) {
        if (std::string("--list") == argv[2]) {
            TestFileReader reader(std::string(argv[1]) + ".in");

            if (!reader.open()) {
                return ExitCode::FailedOpeningTestFile;
            }

            std::optional<::Test::Z80::Test> test;

            while (!reader.eof() && (test = reader.nextTest())) {
                std::cout << test->name() << "\n";
            }

            return ExitCode::Ok;
        }

        int argIdx = 2;
        std::vector<std::string> testNames;

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
