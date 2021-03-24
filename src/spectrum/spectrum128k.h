//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_SPECTRUM128K_H
#define SPECTRUM_SPECTRUM128K_H

#include "basespectrum.h"

namespace Spectrum
{
    /**
     * TODO how are we going to cope with synchronising changes to banks 2 and 5 when they are also paged in? Implement
     *  as custom Z80 subclass that has basic MMU to map reads/writes?
     *
     * TODO IOdevice to handle rom and ram paging
     */
    class Spectrum128k
    : public BaseSpectrum
    {
    public:
        enum class Rom : std::uint8_t
        {
            Rom0,       // ROM for Spectrum 128k
            Rom1,       // ROM for Spectrum 48k
        };

        enum class ScreenBuffer : std::uint8_t
        {
            Normal,       // display the normal screen buffer
            Shadow,       // display the shadow screen buffer
        };

        enum class MemoryBank : std::uint8_t
        {
            Bank0 = 0,
            Bank1,
            Bank2,
            Bank3,
            Bank4,
            Bank5,
            Bank6,
            Bank7,
        };

        static constexpr const int DisplayMemorySize = 6912;

        Spectrum128k();
        explicit Spectrum128k(const std::string & romFile128, const std::string & romFile48);
        ~Spectrum128k() override;

        [[nodiscard]] ::Z80::UnsignedByte * displayMemory() const override;

        [[nodiscard]] int displayMemorySize() const override
        {
            return DisplayMemorySize;
        }

    protected:
        [[nodiscard]] inline Rom currentRom() const
        {
            return m_currentRomNumber;
        }

        void setRom(Rom rom);

        [[nodiscard]] inline MemoryBank pagedMemoryBank() const
        {
            return m_pagedMemoryBank;
        }

        /**
         * The currently paged-in memory.
         *
         * @return
         */
        [[nodiscard]] ::Z80::UnsignedByte * pagedMemory() const;

        /**
         * The cache location of the currently paged-in memory when it's not actually paged in.
         *
         * In other words, where do we store the current memory page when it's paged out.
         *
         * @return
         */
        [[nodiscard]] ::Z80::UnsignedByte * pagedMemoryCache() const;

        void pageMemoryBank(MemoryBank bank);

    private:
        using RomImage = ::Z80::UnsignedByte[0x4000];

        struct RomImages
        {
            RomImage rom[2];
        };
        
        Rom m_currentRomNumber;
        RomImages m_romImages;

        MemoryBank m_pagedMemoryBank;
        ScreenBuffer m_screenBuffer;
    };
}

#endif //SPECTRUM_SPECTRUM128K_H
