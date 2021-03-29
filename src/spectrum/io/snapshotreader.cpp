//
// Created by darren on 22/03/2021.
//

#include <cassert>

#include "snapshotreader.h"

using namespace Spectrum::Io;


SnapshotReader::SnapshotReader(SnapshotReader && other) noexcept
: m_borrowedStream(other.m_borrowedStream),
  m_hasBeenRead(other.m_hasBeenRead),
  m_in(other.m_in),
  m_snapshot(std::move(other.m_snapshot))
{
    other.m_in = nullptr;
    other.m_borrowedStream = false;
}

SnapshotReader::~SnapshotReader()
{
    disposeStream();
}

SnapshotReader & SnapshotReader::operator=(SnapshotReader && other) noexcept
{
    disposeStream();
    m_borrowedStream = other.m_borrowedStream;
    m_hasBeenRead = other.m_hasBeenRead;
    m_in = other.m_in;
    m_snapshot = other.m_snapshot;

    other.m_in = nullptr;
    other.m_borrowedStream = false;

    return *this;
}

void SnapshotReader::disposeStream()
{
    if (!m_borrowedStream) {
        delete m_in;
    }

    m_in = nullptr;
}

void SnapshotReader::setFileName(const std::string & fileName)
{
    disposeStream();
    m_in = new std::ifstream(fileName);
    m_borrowedStream = false;
}

void SnapshotReader::setStream(std::istream * in)
{
    disposeStream();
    m_in = in;
    m_borrowedStream = true;
}

std::istream & SnapshotReader::inputStream() const
{
    assert(m_in);
    return *m_in;
}
