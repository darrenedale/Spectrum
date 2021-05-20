//
// Created by darren on 29/04/2021.
//

#ifndef SPECTRUM_IO_SNAPSHOTFORMATMATCHER_H
#define SPECTRUM_IO_SNAPSHOTFORMATMATCHER_H

// NOTE I think this should be 202002L (publication date) but MSVC has 201907L (feature freeze date)
#if (!defined(__cpp_lib_concepts) || 201907L > __cpp_lib_concepts)
// use polyfill for missing concepts library
#include "../../util/concepts.h"
#endif

namespace Spectrum::Io
{
    /**
     * Concept used for snapshot readers that can indicate whether a file could be a snapshot in their format.
     *
     * Requires a class to implement three static functions and be derived from SnapshotReader in order to satisfy the concept:
     * - const std::string & formatName()
     * - bool couldBeSnapshot(std::istream &)
     * - bool couldBeSnapshot(const std::string & fileName)
     */
    template<class T>
    concept SnapshotFormatMatcher =
            std::derived_from<T, Spectrum::Io::SnapshotReader>
            && requires(T, std::istream & in, const std::string & fileName)
            {
                /**
                 * Provides the arbitrary unique name for the format of snapshot that the reader supports.
                 */
                { T::formatName() } -> std::same_as<const std::string &>;

                /**
                 * Determine whether a stream could be a snapshot in the correct format for the reader.
                 */
                { T::couldBeSnapshot(in) } -> std::convertible_to<bool>;

                /**
                 * Determine whether a file could be a snapshot in the correct format for the reader.
                 */
                { T::couldBeSnapshot(fileName) } -> std::convertible_to<bool>;
            };
}

#endif //SPECTRUM_IO_SNAPSHOTFORMATMATCHER_H
