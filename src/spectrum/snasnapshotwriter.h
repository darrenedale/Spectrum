//
// Created by darren on 15/03/2021.
//

#ifndef SPECTRUM_SNASNAPSHOTWRITER_H
#define SPECTRUM_SNASNAPSHOTWRITER_H

#include "snapshotwriter.h"

namespace Spectrum
{
    /**
     * Write a spectrum snapshot in .sna format.
     *
     * NOTE You must ensure that the PC is on the top of the stack before the snapshot is created otherwise the file
     * will be unusable. Using the following sequence should work:
     *
     *     // execute a CALL $0000 to put the PC on the stack
     *     spectrum.z80()->execute({0xcd, 0x00, 0x00}, false);
     *     Snapshot snap(spectrum);
     *     // execute a RETN to pop the PC from the stack
     *     spectrum.z80()->execute({0xed, 0x45}, false);
     *
     *     // save the snapshot ...
     */
    class SnaSnapshotWriter
    : public SnapshotWriter
    {
    public:
        using SnapshotWriter::SnapshotWriter;

        bool writeTo(std::ostream & out) const override;
        using SnapshotWriter::writeTo;
    };
}

#endif //SPECTRUM_SNASNAPSHOTWRITER_H
