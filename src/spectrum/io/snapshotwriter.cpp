//
// Created by darren on 15/03/2021.
//

#include "snapshotwriter.h"

using namespace Spectrum::Io;
using ::Z80::hostToZ80ByteOrder;

SnapshotWriter::SnapshotWriter(const SnapshotWriter & other)
: m_snapshot(other.m_snapshot ? std::make_unique<Snapshot>(*(other.m_snapshot)) : nullptr)
{}

SnapshotWriter & SnapshotWriter::operator=(const SnapshotWriter & other)
{
    m_snapshot = std::make_unique<Snapshot>(*(other.m_snapshot));
}

void SnapshotWriter::writeHostWord(std::ostream & out, ::Z80::UnsignedWord word)
{
    word = hostToZ80ByteOrder(word);
    out.write(reinterpret_cast<char *>(&word), 2);
}

void SnapshotWriter::writeZ80Word(std::ostream & out, ::Z80::UnsignedWord word)
{
    out.write(reinterpret_cast<char *>(&word), 2);
}
