//
// Created by darren on 15/03/2021.
//

#include "snapshotwriter.h"

using namespace Spectrum::Io;
using ::Z80::hostToZ80ByteOrder;

void SnapshotWriter::writeHostWord(std::ostream & out, ::Z80::UnsignedWord word)
{
    word = hostToZ80ByteOrder(word);
    out.write(reinterpret_cast<char *>(&word), 2);
}

void SnapshotWriter::writeZ80Word(std::ostream & out, ::Z80::UnsignedWord word)
{
    out.write(reinterpret_cast<char *>(&word), 2);
}
