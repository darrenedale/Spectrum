//
// Created by darren on 26/02/2021.
//

#include <iostream>
#include <iomanip>

#include "runner.h"

using namespace Test::Z80;

namespace
{
    std::string formatByte(::Z80::UnsignedByte byte)
    {
        std::ostringstream out;

        if (16 > byte) {
            out << '0';
        }

        out << std::hex << static_cast<std::uint16_t>(byte);
        return out.str();
    }

    std::string formatWord(::Z80::UnsignedWord word)
    {
        if (0 == word) {
            return "0000";
        }

        std::ostringstream out;
        auto padWord = word;

        while (!(0xf000 & padWord)) {
            out << '0';
            padWord <<= 4;
        }

        out << std::hex << word;
        return out.str();
    }

    void logFailures(const Expectation::Failures & failures)
    {
        std::cout << failures.size() << " failure(s)\n";
        auto fill = std::cout.fill();
        auto width = std::cout.width();
        std::cout << std::setfill('0');

        for (const auto & failure : failures) {
            switch (failure.type) {
                case Expectation::FailureType::AfIncorrect:
                    std::cout << "- AF register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::BcIncorrect:
                    std::cout << "- BC register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::DeIncorrect:
                    std::cout << "- DE register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::HlIncorrect:
                    std::cout << "- HL register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::IxIncorrect:
                    std::cout << "- IX register incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::IyIncorrect:
                    std::cout << "- IY register incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::AfShadowIncorrect:
                    std::cout << "- AF' register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::BcShadowIncorrect:
                    std::cout << "- BC' register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::DeShadowIncorrect:
                    std::cout << "- DE' register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::HlShadowIncorrect:
                    std::cout << "- HL' register pair incorrect: expected 0x" << std::hex << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.expected) << ", found 0x" << std::setw(4) << std::any_cast<::Z80::Z80::UnsignedWord>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::IIncorrect:
                    std::cout << "- I register incorrect: expected 0x" << std::hex << std::setw(2) << static_cast<int>(std::any_cast<::Z80::Z80::UnsignedByte>(failure.expected)) << ", found 0x" << std::setw(2) << static_cast<int>(std::any_cast<::Z80::Z80::UnsignedByte>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::RIncorrect:
                    std::cout << "- R register incorrect: expected 0x" << std::hex << std::setw(2) << static_cast<int>(any_cast<::Z80::Z80::UnsignedByte>(failure.expected)) << ", found 0x" << std::setw(2) << static_cast<int>(std::any_cast<::Z80::Z80::UnsignedByte>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::Iff1Incorrect:
                    std::cout << "- IFF1 incorrect: expected " << std::boolalpha << std::any_cast<std::size_t>(failure.expected) << ", found " << std::any_cast<std::size_t>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::Iff2Incorrect:
                    std::cout << "- IFF2 incorrect: expected " << std::boolalpha << std::any_cast<std::size_t>(failure.expected) << ", found " << std::any_cast<std::size_t>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::InterruptModeIncorrect:
                    std::cout << "- interrupt mode incorrect: expected " << std::dec << std::setw(1) << std::any_cast<std::size_t>(failure.expected) << ", found " << std::any_cast<std::size_t>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::MemoryIncorrect:
                    std::cout << "- memory state incorrect: " << failure.message << ".\n";
                    break;

                case Expectation::FailureType::TStatesIncorrect:
                    std::cout << "- t-states incorrect: expected " << std::dec << std::setw(0) << std::any_cast<std::size_t>(failure.expected) << ", found " << std::any_cast<std::size_t>(failure.actual) << ".\n";
                    break;
            }
        }

        std::cout.fill(fill);
        std::cout.width(width);
    }

    int checkOutcome(const Expectation::Failures & failures)
    {
        if (failures.empty()) {
            std::cout << "passed.\n";
        } else {
            logFailures(failures);
        }

        return failures.size();
    }
}

Runner::Runner(std::string testFileBaseName)
: Runner(testFileBaseName, new ::Z80::Z80(new UnsignedByte[65536], 65536))
{
    m_borrowedCpu = false;
}

Runner::Runner(std::string testFileBaseName, ::Z80::Z80 & cpu)
: Runner(testFileBaseName, &cpu)
{
}

Runner::Runner(std::string testFileBaseName, ::Z80::Z80 * cpu)
: m_tests(testFileBaseName),
  m_borrowedCpu(true),
  m_testsRead(false),
  m_cpu(cpu)
{
}

Runner::~Runner()
{
    if (!m_borrowedCpu) {
        delete[] m_cpu->memory();
        delete m_cpu;
    }
};

int Runner::runNextTest()
{
    if (!m_testsRead) {
        m_tests.readTests();
    }

    auto * testCase = m_tests.nextTestCase();

    if (!testCase) {
        return -1;
    }

    return runTest(*testCase);
}

std::tuple<int, int> Runner::runAllTests()
{
    if (!m_testsRead) {
        m_tests.readTests();
    } else {
        m_tests.reset();
    }

    int failureCount = 0;
    int testCount = 0;

    while (m_tests.hasMoreTestCases()) {
        failureCount += runNextTest();
        ++testCount;
    };

    if (0 == testCount) {
        std::cout << "No tests executed.\n";
    } else if (0 == failureCount) {
        std::cout << "All " << testCount << " test(s) passed.\n";
    } else {
        std::cout << failureCount << "failure(s) in " << testCount << " test(s).\n";
    }

    return {testCount, failureCount};
}

void Runner::outputZ80State(std::ostream & stream) const
{
    auto & cpu = this->cpu();
    auto & registers = cpu.registers();

    stream << "        AF   BC   DE   HL   IX   IY   SP   PC  MEMPTR\n";
    stream << "       " << formatWord(registers.af) << ' ' << formatWord(registers.bc) << ' ' << formatWord(registers.de) << ' ' << formatWord(registers.hl) << ' ' << formatWord(registers.ix) << ' ' << formatWord(registers.iy) << ' ' << formatWord(registers.sp) <<  ' ' << formatWord(registers.pc) <<  ' ' << formatWord(registers.memptr) << '\n';
    stream << "Shadow " << formatWord(registers.afShadow) << ' ' << formatWord(registers.bcShadow) << ' ' << formatWord(registers.deShadow) << ' ' << formatWord(registers.hlShadow) << '\n';

    stream << "\n        A  F  B  C  D  E  H  L  I  R\n";
    stream << "       " << formatByte(registers.a()) << ' ' << formatByte(registers.f()) << ' ' << formatByte(registers.b()) << ' ' << formatByte(registers.c()) << ' ' << formatByte(registers.d()) << ' '  << formatByte(registers.e()) << ' ' << formatByte(registers.h()) << ' ' << formatByte(registers.l()) << ' ' << formatByte(registers.i) << ' ' << formatByte(registers.r) << '\n';
    stream << "Shadow " << formatByte(registers.aShadow()) << ' ' << formatByte(registers.fShadow()) << ' ' << formatByte(registers.bShadow()) << ' ' << formatByte(registers.cShadow()) << ' ' << formatByte(registers.dShadow()) << ' '  << formatByte(registers.eShadow()) << ' ' << formatByte(registers.hShadow()) << ' ' << formatByte(registers.lShadow()) << '\n';

    stream << "\n       IFF1 IFF2 IM\n";
    stream << "         " << (cpu.iff1() ? '1' : '0') << "    " << (cpu.iff2() ? '1' : '0') << "   " << cpu.interruptMode() << '\n';
}

/**
 * Run a single test by name.
 *
 * @param description
 * @return
 */
bool Runner::runTest(const std::string & description)
{
    if (!m_testsRead) {
        m_tests.readTests();
    }

    const auto & testCase = m_tests.testCases().find(description);

    if (testCase == m_tests.testCases().cend()) {
        return false;
    }

    auto failureCount = runTest(testCase->second);

    if (0 == failureCount) {
        std::cout << "Test passed.\n";
    } else {
        std::cout << failureCount << "failure(s) in 1 test.\n";
    }

    return true;
}

/**
 * Run a single test by name.
 *
 * @param description
 * @return
 */
std::tuple<int, int> Runner::runTests(const std::vector<std::string> & tests)
{
    if (!m_testsRead) {
        m_tests.readTests();
    }

    int failureCount = 0;
    int testCount = 0;

    for (const auto & name : tests) {
        const auto & testCase = m_tests.testCases().find(name);

        if (testCase == m_tests.testCases().cend()) {
            std::cerr << "Test named '" << name << "' not found.\n";
            continue;
        }

        failureCount += runTest(testCase->second);
        ++testCount;
    }

    if (0 == testCount) {
        std::cout << "No tests executed.\n";
    } else if (0 == failureCount) {
        std::cout << "All " << testCount << " test(s) passed.\n";
    } else {
        std::cout << failureCount << " failure(s) in " << testCount << " test(s).\n";
    }

    return {testCount, failureCount};
}

/**
 * Internal helper to run a specific test.
 *
 * NOTE Event expectations are not checked since the Z80 emulator intentionally does not attempt to be cycle-exact. The
 * overall time spent on a set of instructions is, however, checked.
 *
 * @param testCase
 */
int Runner::runTest(const TestBattery::TestCase & testCase)
{
    // perform the test
    auto & test = testCase.test;
    int failureCount = 0;

    std::cout << "============================================================\n";
    std::cout << "Test: " << test.name() << "\n";
    std::cout << "------------------------------------------------------------\n";
    test.setupZ80(cpu());
    test.setupMemory(cpu().memory());

    std::uint64_t tStates = 0;

    while (test.tStates() > tStates) {
        tStates += cpu().fetchExecuteCycle();
    }

    std::cout << "t-states: " << tStates << "\n\n";
    outputZ80State(std::cout);
    std::cout << "------------------------------------------------------------\n";

    // check outcome against expectations
    if (!testCase.expectation) {
        std::cout << "No test expectations set.\n";
    } else {
        auto & expectation = *(testCase.expectation);
        std::cout << "Register pairs: ";
        failureCount += checkOutcome(expectation.checkRegisterPairs(cpu()));
        std::cout << "Shadow register pairs: ";
        failureCount += checkOutcome(expectation.checkShadowRegisterPairs(cpu()));
        std::cout << "Registers: ";
        failureCount += checkOutcome(expectation.checkRegisters(cpu()));
        std::cout << "Interrupt Flip-Flops: ";
        failureCount += checkOutcome(expectation.checkInterruptFlipFlops(cpu()));
        std::cout << "Interrupt mode: ";
        failureCount += checkOutcome(expectation.checkInterruptMode(cpu()));
        std::cout << "Memory: ";
        failureCount += checkOutcome(expectation.checkMemory(cpu()));
        std::cout << "T-states: ";
        failureCount += checkOutcome(expectation.checkTStates(tStates));
    }

    std::cout << "============================================================\n";
    std::cout << '\n' << std::flush;
    return failureCount;
}
