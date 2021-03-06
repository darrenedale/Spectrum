//
// Created by darren on 22/03/2021.
//

#include <cassert>
#include "snapshotreader.h"

using namespace Spectrum::Io;

SnapshotReader::SnapshotReader(SnapshotReader && other) noexcept
: m_borrowedStream(other.m_borrowedStream),
  m_in(other.m_in),
  m_snapshot(std::move(other.m_snapshot))
{
    other.m_in = nullptr;
    other.m_borrowedStream = false;
}

SnapshotReader & SnapshotReader::operator=(SnapshotReader && other) noexcept
{
    disposeStream();
    m_borrowedStream = other.m_borrowedStream;
    m_in = other.m_in;
    m_snapshot = std::move(other.m_snapshot);

    other.m_in = nullptr;
    other.m_borrowedStream = false;

    return *this;
}

SnapshotReader::~SnapshotReader()
{
    disposeStream();
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
    m_in = new std::ifstream(fileName, std::ios::binary | std::ios::in);
    m_snapshot.reset();
    m_borrowedStream = false;
}

void SnapshotReader::setStream(std::istream & in)
{
    disposeStream();
    m_in = &in;
    m_snapshot.reset();
    m_borrowedStream = true;
}

void SnapshotReader::setStream(std::istream * in)
{
    disposeStream();
    m_in = in;
    m_snapshot.reset();
    m_borrowedStream = true;
}

void SnapshotReader::setStream(std::unique_ptr<std::istream> in)
{
    disposeStream();
    m_in = in.release();
    m_borrowedStream = false;
    m_snapshot.reset();
}

std::istream & SnapshotReader::inputStream() const
{
    assert(m_in);
    return *m_in;
}
