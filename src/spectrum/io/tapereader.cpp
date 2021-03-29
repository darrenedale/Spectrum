//
// Created by darren on 29/03/2021.
//

#include "tapereader.h"

using namespace Spectrum::Io;

TapeReader::TapeReader(TapeReader && other) noexcept
: m_borrowedStream(other.m_borrowedStream),
  m_in(other.m_in)
{
    other.m_in = nullptr;
    other.m_borrowedStream = false;
}

TapeReader::~TapeReader()
{
    disposeStream();
}

TapeReader & TapeReader::operator=(TapeReader && other) noexcept
{
    disposeStream();
    m_borrowedStream = other.m_borrowedStream;
    m_in = other.m_in;

    other.m_in = nullptr;
    other.m_borrowedStream = false;

    return *this;
}

void TapeReader::disposeStream()
{
    if (!m_borrowedStream) {
        delete m_in;
    }

    m_in = nullptr;
}

void TapeReader::setFileName(const std::string & fileName)
{
    disposeStream();
    m_in = new std::ifstream(fileName);
    m_borrowedStream = false;
}

void TapeReader::setStream(std::istream * in)
{
    disposeStream();
    m_in = in;
    m_borrowedStream = true;
}

std::istream & TapeReader::inputStream() const
{
    assert(m_in);
    return *m_in;
}
