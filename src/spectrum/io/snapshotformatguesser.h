//
// Created by darren on 29/04/2021.
//

#ifndef SPECTRUM_IO_SNAPSHOTFORMATGUESSER_H
#define SPECTRUM_IO_SNAPSHOTFORMATGUESSER_H

#include <optional>
#include "snapshotformatmatcher.h"

namespace Spectrum::Io
{
    /**
     * Helper class to guess the format of a snapshot file/stream from amongst a cohort of possible formats.
     *
     * Provide the template with all the reader classes that you want to check against the file/stream in order of preference, and the guessFormat() function
     * will return the formatName() for the first one that says the file/stream might be a snapshot in that format. Each of the reader classes provided must
     * satisfy the concept SnapshotFormatMatcher.
     *
     * For example:
     *
     * SnapshotFormatGuesser<Z80SnapshotReader, SnaSnapshotReader>::guessFormat("snapshot_file");
     *
     * will return:
     * - "z80" if the Z80SnapshotReader says it looks like a Z80 snapshot;
     * - "sna" if the Z80SnapshotReader says it doesn't look like a Z80 snapshot and the SnaSnapshotReader says it looks like a SNA snapshot;
     * - an empty optional if neither reader likes the look of the file (or it can't be opened or read, etc.)
     */
    template<SnapshotFormatMatcher T, SnapshotFormatMatcher... Ts>
    class SnapshotFormatGuesser
    {
    public:
        /**
         * Attempt to guess the format of a snapshot file.
         *
         * @param fileName The path to the file.
         *
         * @return The identifier of the format, or an empty optional if none of the available formats matches.
         */
        static std::optional<std::string> guessFormat(const std::string & fileName)
        {
            if (T::couldBeSnapshot(fileName)) {
                return T::formatName();
            }

            if constexpr (0 < sizeof...(Ts)) {
                return SnapshotFormatGuesser<Ts...>::guessFormat(fileName);
            }

            return {};
        }

        /**
         * Attempt to guess the snapshot format for a stream's content.
         *
         * The input stream is assumed to be positioned at the start of the snapshot data.
         *
         * @param in The input stream.
         *
         * @return The identifier of the format, or an empty optional if none of the available formats matches.
         */
        static std::optional<std::string> guessFormat(std::istream & in)
        {
            if (T::couldBeSnapshot(in)) {
                return T::formatName();
            }

            if constexpr (0 < sizeof...(Ts)) {
                return SnapshotFormatGuesser<Ts...>::guessFormat(in);
            }

            return {};
        }
    };
}

#endif //SPECTRUM_IO_SNAPSHOTFORMATGUESSER_H
