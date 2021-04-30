//
// Created by darren on 29/04/2021.
//

#ifndef SPECTRUM_IO_SNAPSHOTWRITERFACTORY_H
#define SPECTRUM_IO_SNAPSHOTWRITERFACTORY_H

#include "snapshotwriterclass.h"

namespace Spectrum::Io
{
    /**
     * Factory to produce a SnapshotWriter instance for a named snapshot format.
     *
     * Provide the candidate SnapshotWriter classes as template arguments. Each class provided must satisfy the SnapshotWriterClass concept in order to be used
     * with the factory. A call to the static writerForFormat() method will produce a SnapshotWriter object if the format requested is provided by one of the
     * SnapshotWriter subclasses provided in the template arguments.
     *
     * The template cannot be instantiated without at least one SnapshotWriter subclass as an argument. The writer classes should be provided in order of
     * preference: the first one that is able to write the requested format will be chosen. There is no harm in providing the same writer subclass multiple
     * times as a template argument; but there's also no value in doing so, so you should avoid this.
     *
     * @tparam T The preferred writer class.
     * @tparam Ts The other writer classes, in order of preference.
     */
    template<SnapshotWriterClass T, SnapshotWriterClass... Ts>
    class SnapshotWriterFactory
    {
    public:
        /**
         * Fetch the appropriate writer for the requested format from those available in the cohort provided to the class template.
         *
         * @param format The snapshot format name.
         *
         * @return A pointer to a snapshot writer, or nullptr if no writer can be located for the format.
         */
        static std::unique_ptr<SnapshotWriter> writerForFormat(const std::string & format)
        {
            if (T::formatName() == format) {
                return std::make_unique<T>();
            }

            if constexpr (0 < sizeof...(Ts)) {
                return SnapshotWriterFactory<Ts...>::writerForFormat(format);
            }

            return nullptr;
        }
    };
}

#endif //SPECTRUM_IO_SNAPSHOTWRITERFACTORY_H
