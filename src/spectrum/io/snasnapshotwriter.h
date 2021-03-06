//
// Created by darren on 15/03/2021.
//

#ifndef SPECTRUM_IO_SNASNAPSHOTWRITER_H
#define SPECTRUM_IO_SNASNAPSHOTWRITER_H

#include "snapshotwriter.h"

namespace Spectrum::Io
{
    /**
     * Write a spectrum snapshot in .sna format.
     *
     * There are two variations of this format, one for 48K Spectrums, one for 128K Spectrums. This writer currently only supports the 48K format.
     *
     * NOTE You must ensure that the PC is on the top of the stack before the snapshot is created otherwise the file will be unusable. Using the following
     * sequence should work:
     *
     *     // execute a CALL $0000 to put the PC on the stack
     *     spectrum.z80()->execute({0xcd, 0x00, 0x00}, false);
     *     Snapshot snap(spectrum);
     *     // execute a RETN to pop the PC from the stack
     *     spectrum.z80()->execute({0xed, 0x45}, false);
     *
     *     // save the snapshot ...
     *
     * This is a quirk of the .sna format - it doesn't store the PC in the snapshot, it expects the PC to be on the top of the Spectrum stack, and expects a
     * RETN instruction to be executed to when the snapshot is restored to a Spectrum.
     *
     * Satisfies the SnapshotWriterClass concept.
     */
    class SnaSnapshotWriter
    : public SnapshotWriter
    {
    public:
        /**
         * Import the base class constructor.
         */
        using SnapshotWriter::SnapshotWriter;

        /**
         * Fetch the name for the format written by this writer.
         *
         * Note that we could theoretically make this constexpr - it's in the C++20 standard but is not supported yet in most compilers.
         *
         * @return The string "sna".
         */
        static const std::string & formatName()
        {
            static const std::string name("sna");
            return name;
        }

        /**
         * Write the snapshot in .sna format to the provided stream.
         *
         * @param out The stream to write the snapshot to.
         *
         * @return
         */
        bool writeTo(std::ostream & out) const override;

        /**
         * Import the other overloads of writeTo() from the base class.
         */
        using SnapshotWriter::writeTo;
    };
}

#endif //SPECTRUM_IO_SNASNAPSHOTWRITER_H
