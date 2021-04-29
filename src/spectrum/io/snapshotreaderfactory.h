//
// Created by darren on 29/04/2021.
//

#ifndef SPECTRUM_IO_SNAPSHOTREADERFACTORY_H
#define SPECTRUM_IO_SNAPSHOTREADERFACTORY_H

#include "snapshotreader.h"

namespace Spectrum::Io
{
    /**
     * A factory to create a reader for a snapshot file or stream.
     *
     * The factory receives the possible reader types as template arguments, in order of preference. The first reader that is found that indicates it thinks it
     * can read the snapshot will be instantiated with the given file/stream. At least one reader class is required by the template.
     *
     * @tparam T The first reader class to try.
     * @tparam Ts The other reader classes to try, in order of preference.
     */
    template<SnapshotFormatMatcher T, SnapshotFormatMatcher... Ts>
    class SnapshotReaderFactory
    {
    public:
        /**
         * Attempt to create a snapshot reader for the given file.
         *
         * If successful, the reader will be created with the file ready to read.
         *
         * @param fileName The path to the file.
         *
         * @return A reader to read the snapshot file, or nullptr if no appropriate reader could be created.
         */
        static std::unique_ptr<SnapshotReader> readerFor(const std::string & fileName)
        {
            if (T::couldBeSnapshot(fileName)) {
                return std::make_unique<T>(fileName);
            }

            if constexpr (0 < sizeof...(Ts)) {
                return SnapshotReaderFactory<Ts...>::readerFor(fileName);
            }

            return nullptr;
        }

        /**
         * Attempt to create a snapshot reader for the given stream.
         *
         * The input stream is assumed to be positioned at the start of the snapshot data.
         *
         * @param in The input stream.
         *
         * @return A reader to read the snapshot data from the stream, or nullptr if no appropriate reader could be created.
         */
        static std::unique_ptr<SnapshotReader> readerFor(std::istream & in)
        {
            auto pos = in.tellg();

            if (T::couldBeSnapshot(in)) {
                in.seekg(pos);
                return std::make_unique<T>(in);
            }

            in.seekg(pos);

            if constexpr (0 < sizeof...(Ts)) {
                return SnapshotReaderFactory<Ts...>::readerFor(in);
            }

            return nullptr;
        }

        /**
         * Attempt to create a reader for a specified snapshot format for the given file.
         *
         * @param format The snapshot format name.
         * @param input The the file name to read.
         *
         * @return
         */
        static std::unique_ptr<SnapshotReader> readerForFormat(const std::string & format, const std::string & fileName)
        {
            if (T::formatName() == format) {
                return std::make_unique<T>(fileName);
            }

            if constexpr (0 < sizeof...(Ts)) {
                return SnapshotReaderFactory<Ts...>::readerForFormat(format, fileName);
            }

            return nullptr;
        }

        /**
         * Attempt to create a reader for a specified snapshot format for the given input stream.
         *
         * @param format The snapshot format name.
         * @param in The the input stream to read.
         *
         * @return
         */
        static std::unique_ptr<SnapshotReader> readerForFormat(const std::string & format, std::istream & in)
        {
            if (T::formatName() == format) {
                return std::make_unique<T>(in);
            }

            if constexpr (0 < sizeof...(Ts)) {
                return SnapshotReaderFactory<Ts...>::readerForFormat(format, in);
            }

            return nullptr;
        }
    };
}

#endif //SPECTRUM_IO_SNAPSHOTREADERFACTORY_H
