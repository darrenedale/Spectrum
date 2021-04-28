//
// Created by darren on 27/04/2021.
//

#ifndef SPECTRUM_IO_ZXSNAPSHOTWRITER_H
#define SPECTRUM_IO_ZXSNAPSHOTWRITER_H

#include "snapshotwriter.h"
#include "../../z80/types.h"

namespace Spectrum::Io
{
    /**
     * Write a snapshot in .zx (Amiga KGB emulator) format.
     *
     * This isn't a great format for interchange - as far as I can tell the screen border colour is not stored and there's plenty of wasted space in the
     * "header" that stores the registers etc. It also stores the memory image before the "header" so it can't be extended to support 128K Spectrums, and it
     * stores 132 bytes of ROM (the last 16.5 characters in the character set) with the memory image.
     */
    class ZxSnapshotWriter
    : public SnapshotWriter
    {
    public:
        /**
         * Import the base class constructors.
         */
        using SnapshotWriter::SnapshotWriter;

        /**
         * Write the snapshot to the provided stream.
         *
         * @param out The stream to write to.
         *
         * @return true if the write succeeded, false if not.
         */
        bool writeTo(std::ostream & out) const override;

        /**
         * Import remaining overloads of writeTo() from base class.
         */
        using SnapshotWriter::writeTo;

    };
}

#endif //SPECTRUM_IO_ZXSNAPSHOTWRITER_H
