//
// Created by darren on 29/04/2021.
//

#ifndef SPECTRUM_IO_SNAPSHOTWRITERCLASS_H
#define SPECTRUM_IO_SNAPSHOTWRITERCLASS_H

#include "snapshotwriter.h"

namespace Spectrum::Io
{
    /**
     * Concept used for snapshot writers that provide a class-specific static identifier of the snapshot format they write.
     *
     * Requires a class to implement one static function and be derived from SnapshotWriter in order to satisfy the concept:
     * - const std::string & formatName()
     */
    template<class T>
    concept SnapshotWriterClass =
        std::derived_from<T, SnapshotWriter> &&
        requires(T)
    {
        /**
         * Provides the arbitrary unique name for the format of snapshot that the writer produces.
         */
        {T::formatName()} -> std::same_as<const std::string &>;
    };
}

#endif //SPECTRUM_IO_SNAPSHOTWRITERCLASS_H
