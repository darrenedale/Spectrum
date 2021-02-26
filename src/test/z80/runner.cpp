//
// Created by darren on 26/02/2021.
//

#include "runner.h"

using namespace Spectrum::Test::Z80;

Runner::Runner(std::string testFile)
: Runner(testFile, new ::Z80(new UnsignedByte[65536], 65536))
{
    m_borrowedCpu = false;
}

Runner::Runner(std::string testFile, ::Z80 & cpu)
: Runner(testFile, &cpu)
{
}

Runner::Runner(std::string testFile, ::Z80 * cpu)
: m_reader(testFile),
  m_borrowedCpu(true),
  m_cpu(cpu),
  m_currentTest()
{
}

Spectrum::Test::Z80::Runner::~Runner()
{
    if (!m_borrowedCpu) {
        delete[] m_cpu->memory();
        delete m_cpu;
    }
};

void Runner::runNextTest()
{
    m_currentTest = std::move(m_reader.nextTest());

    if (!m_currentTest) {
        return;
    }

    auto & test = *m_currentTest;
    test.setupZ80(cpu());
    test.setupMemory(cpu().memory());

    // TODO run test

    // TODO check expectations
}

void Runner::runAllTests()
{
    do {
        runNextTest();
    } while (m_currentTest);
}
