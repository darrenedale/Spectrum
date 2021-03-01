//
// Created by darren on 26/02/2021.
//

#include <iostream>
#include <iomanip>

#include "runner.h"

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
}

using namespace Test::Z80;

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

namespace
{

    void logFailures(const Expectation::Failures & failures)
    {
        std::cout << failures.size() << " failure(s)\n";

        for (const auto & failure : failures) {
            switch (failure) {
                case ExpectationFailure::AfIncorrect:
                    std::cout << "- Incorrect AF register pair state found.\n";
                    break;

                case ExpectationFailure::BcIncorrect:
                    std::cout << "- Incorrect BC register pair state found.\n";
                    break;

                case ExpectationFailure::DeIncorrect:
                    std::cout << "- Incorrect DE register pair state found.\n";
                    break;

                case ExpectationFailure::HlIncorrect:
                    std::cout << "- Incorrect HL register pair state found.\n";
                    break;

                case ExpectationFailure::IxIncorrect:
                    std::cout << "- Incorrect IX register state found.\n";
                    break;

                case ExpectationFailure::IyIncorrect:
                    std::cout << "- Incorrect IY register state found.\n";
                    break;

                case ExpectationFailure::AfShadowIncorrect:
                    std::cout << "- Incorrect AF' register pair state found.\n";
                    break;

                case ExpectationFailure::BcShadowIncorrect:
                    std::cout << "- Incorrect BC' register pair state found.\n";
                    break;

                case ExpectationFailure::DeShadowIncorrect:
                    std::cout << "- Incorrect DE' register pair state found.\n";
                    break;

                case ExpectationFailure::HlShadowIncorrect:
                    std::cout << "- Incorrect HL' register pair state found.\n";
                    break;

                case ExpectationFailure::IIncorrect:
                    std::cout << "- Incorrect I register state found.\n";
                    break;

                case ExpectationFailure::RIncorrect:
                    std::cout << "- Incorrect R register state found.\n";
                    break;

                case ExpectationFailure::Iff1Incorrect:
                    std::cout << "- Incorrect IFF1 state found.\n";
                    break;

                case ExpectationFailure::Iff2Incorrect:
                    std::cout << "- Incorrect IFF2 state found.\n";
                    break;

                case ExpectationFailure::InterruptModeIncorrect:
                    std::cout << "- Incorrect interrupt mode found.\n";
                    break;

                case ExpectationFailure::MemoryIncorrect:
                    std::cout << "- Expected memory state not found.\n";
                    break;
            }
        }
    }

    void checkOutcome(const Expectation::Failures & failures)
    {
        if (failures.empty()) {
            std::cout << "passed.\n";
        } else {
            logFailures(failures);
        }
    }
}

void Runner::runNextTest()
{
    if (!m_testsRead) {
        m_tests.readTests();
    }

    auto * testCase = m_tests.nextTestCase();

    if (!testCase) {
        return;
    }

    runTest(*testCase);
}

void Runner::runAllTests()
{
    if (!m_testsRead) {
        m_tests.readTests();
    } else {
        m_tests.reset();
    }

    while (m_tests.hasMoreTestCases()) {
        runNextTest();
    };
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

bool Runner::runTest(const std::string & description)
{
    if (!m_testsRead) {
        m_tests.readTests();
    }

    const auto & testCase = m_tests.testCases().find(description);

    if (testCase == m_tests.testCases().cend()) {
        return false;
    }

    runTest(testCase->second);
    return true;
}

/**
 * NOTE Event expectations are not checked since the Z80 emulator intentionally does not attempt to be cycle-exact. The
 * overall time spent on a set of instructions is, however, checked.
 *
 * @param testCase
 */
void Runner::runTest(const TestBattery::TestCase & testCase)
{
    // perform the test
    auto & test = testCase.test;

    std::cout << "============================================================\n";
    std::cout << "Test: " << test.description() << "\n";
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
        checkOutcome(expectation.checkRegisterPairs(cpu()));
        std::cout << "Shadow register pairs: ";
        checkOutcome(expectation.checkShadowRegisterPairs(cpu()));
        std::cout << "Registers: ";
        checkOutcome(expectation.checkRegisters(cpu()));
        std::cout << "Interrupt Flip-Flops: ";
        checkOutcome(expectation.checkInterruptFlipFlops(cpu()));
        std::cout << "Interrupt mode: ";
        checkOutcome(expectation.checkInterruptMode(cpu()));
        std::cout << "Memory: ";
        checkOutcome(expectation.checkMemory(cpu()));
        // TODO check t-states
    }

    std::cout << "============================================================\n";
    std::cout << '\n' << std::flush;
}
