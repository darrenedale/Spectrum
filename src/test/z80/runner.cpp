//
// Created by darren on 26/02/2021.
//

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include "runner.h"
#include "../../z80/types.h"

using namespace Test::Z80;

namespace
{
    /**
     * Helper to format a byte for output.
     *
     * @param byte
     * @return
     */
    std::string formatByte(::Z80::UnsignedByte byte)
    {
        std::ostringstream out;
        out << std::hex << std::setfill('0') << std::setw(2) << static_cast<UnsignedWord>(byte);
        return out.str();
    }

    /**
     * Helper to format a 16-bit word for output.
     *
     * @param byte
     * @return
     */
    std::string formatWord(::Z80::UnsignedWord word)
    {
        std::ostringstream out;
        out << std::hex << std::setfill('0') << std::setw(4) << word;
        return out.str();
    }

    /**
     * Helper to log a set of failures to standard output.
     *
     * @param failures
     */
    void logFailures(const Expectation::Failures & failures)
    {
        std::cout << failures.size() << " failure(s)\n";
        auto width = std::cout.width();

        for (const auto & failure : failures) {
            switch (failure.type) {
                case Expectation::FailureType::AfIncorrect:
                    std::cout << "- AF register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::BcIncorrect:
                    std::cout << "- BC register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::DeIncorrect:
                    std::cout << "- DE register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::HlIncorrect:
                    std::cout << "- HL register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::IxIncorrect:
                    std::cout << "- IX register incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::IyIncorrect:
                    std::cout << "- IY register incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::AfShadowIncorrect:
                    std::cout << "- AF' register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::BcShadowIncorrect:
                    std::cout << "- BC' register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::DeShadowIncorrect:
                    std::cout << "- DE' register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::HlShadowIncorrect:
                    std::cout << "- HL' register pair incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::MemptrIncorrect:
                    std::cout << "- MEMPTR register incorrect: expected 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.expected)) << ", found 0x" << formatWord(std::any_cast<::Z80::UnsignedWord>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::IIncorrect:
                    std::cout << "- I register incorrect: expected 0x" << formatByte(std::any_cast<::Z80::UnsignedByte>(failure.expected)) << ", found 0x" << formatByte(std::any_cast<::Z80::UnsignedByte>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::RIncorrect:
                    std::cout << "- R register incorrect: expected 0x" << formatByte(std::any_cast<::Z80::UnsignedByte>(failure.expected)) << ", found 0x" << formatByte(std::any_cast<::Z80::UnsignedByte>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::Iff1Incorrect:
                    std::cout << "- IFF1 incorrect: expected " << std::boolalpha << std::any_cast<bool>(failure.expected) << ", found " << std::any_cast<bool>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::Iff2Incorrect:
                    std::cout << "- IFF2 incorrect: expected " << std::boolalpha << std::any_cast<bool>(failure.expected) << ", found " << std::any_cast<bool>(failure.actual) << ".\n";
                    break;

                case Expectation::FailureType::InterruptModeIncorrect:
                    std::cout << "- interrupt mode incorrect: expected " << std::setw(1) << static_cast<std::uint16_t>(std::any_cast<InterruptMode>(failure.expected)) << ", found " << static_cast<std::uint16_t>(std::any_cast<InterruptMode>(failure.actual)) << ".\n";
                    break;

                case Expectation::FailureType::MemoryIncorrect:
                    std::cout << "- memory state incorrect: " << failure.message << ".\n";
                    break;

                case Expectation::FailureType::TStatesIncorrect:
                    std::cout << "- t-states incorrect: expected " << std::dec << std::setw(0) << std::any_cast<std::size_t>(failure.expected) << ", found " << std::any_cast<std::size_t>(failure.actual) << ".\n";
                    break;
            }
        }

        std::cout.width(width);
    }

    /**
     * Helper to check an outcome and write a suitable message to standard output.
     *
     * @param failures
     * @return
     */
    int checkOutcome(const Expectation::Failures & failures)
    {
        if (failures.empty()) {
            std::cout << "passed.\n";
        } else {
            logFailures(failures);
        }

        return static_cast<int>(failures.size());
    }
}

Runner::Runner(std::string testFileBaseName)
: Runner(std::move(testFileBaseName), new ::Z80::Z80(new SimpleMemory(65536)))
{
    m_borrowedCpu = false;
    m_ioDevice = std::make_unique<TestIoDevice>();
    m_cpu->connectIODevice(m_ioDevice.get());
}

Runner::Runner(std::string testFileBaseName, ::Z80::Z80 & cpu)
: Runner(std::move(testFileBaseName), &cpu)
{
}

Runner::Runner(std::string testFileBaseName, ::Z80::Z80 * cpu)
: m_tests(std::move(testFileBaseName)),
  m_borrowedCpu(true),
  m_testsRead(false),
  m_cpu(cpu),
  m_onlyOutputFailedTests(false)
{
}

Runner::~Runner()
{
    if (!m_borrowedCpu) {
        delete[] m_cpu->memory();
        delete m_cpu;
    }
}

int Runner::runNextTest()
{
    if (!m_testsRead) {
        m_tests.readTests();
        m_testsRead = true;
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
        m_testsRead = true;
    } else {
        m_tests.reset();
    }

    int failureCount = 0;
    int testCount = 0;

    while (m_tests.hasMoreTestCases()) {
        failureCount += runNextTest();
        ++testCount;
    }

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
    stream << "       " << formatByte(registers.a) << ' ' << formatByte(registers.f) << ' ' << formatByte(registers.b) << ' ' << formatByte(registers.c) << ' ' << formatByte(registers.d) << ' '  << formatByte(registers.e) << ' ' << formatByte(registers.h) << ' ' << formatByte(registers.l) << ' ' << formatByte(registers.i) << ' ' << formatByte(registers.r) << '\n';
    stream << "Shadow " << formatByte(registers.aShadow) << ' ' << formatByte(registers.fShadow) << ' ' << formatByte(registers.bShadow) << ' ' << formatByte(registers.cShadow) << ' ' << formatByte(registers.dShadow) << ' '  << formatByte(registers.eShadow) << ' ' << formatByte(registers.hShadow) << ' ' << formatByte(registers.lShadow) << '\n';

    stream << "\n       IFF1 IFF2 IM\n";
    stream << "         " << (cpu.iff1() ? '1' : '0') << "    " << (cpu.iff2() ? '1' : '0') << "   " << static_cast<std::uint16_t>(cpu.interruptMode()) << '\n';
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
        m_testsRead = true;
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
        m_testsRead = true;
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
    cpu().reset();
    cpu().memory()->clear();
    test.setupZ80(cpu());
    test.setupMemory(*(cpu().memory()));

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
        // TODO when MEMPTR is fully supported in Z80 class, check MEMPTR expectation
        auto & expectation = *(testCase.expectation);
        std::cout << "Register pairs: ";
        failureCount += checkOutcome(expectation.checkRegisterPairs(cpu()));
        std::cout << "Shadow register pairs: ";
        failureCount += checkOutcome(expectation.checkShadowRegisterPairs(cpu()));
//        std::cout << "MEMPTR: ";
//        failureCount += checkOutcome(expectation.checkMemptr(cpu()));
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
        std::cout << "------------------------------------------------------------\n";
        std::cout << "Result: " << (0 == failureCount ? "pass" : "fail") << '\n';
    }

    std::cout << "============================================================\n";
    std::cout << '\n' << std::flush;
    return failureCount;
}
