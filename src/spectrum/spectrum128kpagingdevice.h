//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_SPECTRUM128KPAGINGDEVICE_H
#define SPECTRUM_SPECTRUM128KPAGINGDEVICE_H

#include "../z80/iodevice.h"
#include "../z80/types.h"

namespace Spectrum
{
    class Spectrum128k;

    /**
     * Z80 IO device to handle ROM and RAM paging for the Spectrum 128k.
     */
    class Spectrum128KPagingDevice
    : public ::Z80::IODevice
    {
    public:
        explicit Spectrum128KPagingDevice(Spectrum128k & owner)
        : m_pagingEnabled(true),
          m_spectrum(owner)
        {}

        [[nodiscard]] inline bool checkReadPort(::Z80::UnsignedWord port) const override
        {
            return false;
        }

        [[nodiscard]] inline bool checkWritePort(::Z80::UnsignedWord port) const override
        {
            // respond if bits 2 and 15 are both set low in the port, other bits are irrelevant
            return (0b0111111111111101 == (port | 0b0111111111111101));
        }

        Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override
        {
            return 0xff;
        }

        void writeByte(::Z80::UnsignedWord port, ::Z80::UnsignedByte value) override;

        [[nodiscard]] const Spectrum128k & spectrum() const
        {
            return m_spectrum;
        }

        Spectrum128k & spectrum()
        {
            return m_spectrum;
        }

        [[nodiscard]] bool pagingEnabled() const
        {
            return m_pagingEnabled;
        }

        void reset()
        {
            setPagingEnabled(true);
        }

    protected:
        void setPagingEnabled(bool enabled)
        {
            m_pagingEnabled = enabled;
        }

    private:
        bool m_pagingEnabled;
        Spectrum128k & m_spectrum;
    };
}

#endif //SPECTRUM_SPECTRUM128KPAGINGDEVICE_H
