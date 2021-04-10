//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_IO_TAPEREADER_H
#define SPECTRUM_IO_TAPEREADER_H

#include <istream>

namespace Spectrum::Io
{
    class TapeReader
    {
    public:
        explicit TapeReader(std::istream * in)
        : m_borrowedStream(true),
          m_in(in)
        {}

        explicit TapeReader(const std::string & inFile)
        : m_borrowedStream(false),
          m_in(new std::ifstream(inFile, std::ios::binary | std::ios::in))
        {}

        TapeReader(const SnapshotReader & other) = delete;
        TapeReader(SnapshotReader && other) noexcept;
        TapeReader & operator=(const SnapshotReader & other) = delete;
        TapeReader & operator=(SnapshotReader && other) noexcept;

        virtual ~TapeReader();

        void setFileName(const std::string & fileName);
        void setStream(std::istream * in);

        [[nodiscard]] bool isOpen() const
        {
            return m_in && *m_in;
        }

    protected:
        void disposeStream();
        std::istream & inputStream() const;

    private:
        bool m_borrowedStream;
        std::istream * m_in;
    };
}

#endif //SPECTRUM_IO_TAPEREADER_H
