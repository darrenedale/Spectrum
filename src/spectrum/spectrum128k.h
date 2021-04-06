//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_SPECTRUM128K_H
#define SPECTRUM_SPECTRUM128K_H

#include "basespectrum.h"
#include "spectrum128kmemory.h"
#include "spectrum128kpagingdevice.h"

namespace Spectrum
{
    class Spectrum128k
    : public BaseSpectrum
    {
    public:
        enum class ScreenBuffer : std::uint8_t
        {
            Normal,       // display the normal screen buffer
            Shadow,       // display the shadow screen buffer
        };

        static constexpr const int DisplayMemorySize = 6912;

        Spectrum128k();
        explicit Spectrum128k(const std::string & romFile0, const std::string & romFile1);
        ~Spectrum128k() override;

        [[nodiscard]] ::Z80::UnsignedByte * displayMemory() const override;

        [[nodiscard]] int displayMemorySize() const override
        {
            return DisplayMemorySize;
        }

        [[nodiscard]] inline ScreenBuffer screenBuffer() const
        {
            return m_screenBuffer;
        }

        inline void setScreenBuffer(ScreenBuffer buffer)
        {
            m_screenBuffer = buffer;
        }

        void reset() override;

    protected:
        void reloadRoms() override;

        [[nodiscard]] inline Spectrum128KMemory * memory128() const
        {
            return dynamic_cast<Spectrum128KMemory *>(memory());
        }

    private:
        Spectrum128KPagingDevice m_pager;
        ScreenBuffer m_screenBuffer;
        std::string m_romFiles[2];
    };
}

#endif //SPECTRUM_SPECTRUM128K_H
