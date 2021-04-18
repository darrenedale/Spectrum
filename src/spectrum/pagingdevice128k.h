//
// Created by darren on 06/04/2021.
//

#ifndef SPECTRUM_PAGINGDEVICE128K_H
#define SPECTRUM_PAGINGDEVICE128K_H

#include "../z80/iodevice.h"
#include "../z80/types.h"

namespace Spectrum
{
    class Spectrum128k;

    /**
     * Z80 IO device to handle ROM and RAM paging for the Spectrum 128k.
     */
    class PagingDevice128k
    : public ::Z80::IODevice
    {
    public:
        /**
         * Initialise a device to handle paging for the provided Spectrum 128K.
         *
         * The caller is required to ensure that the paging device does not outlive the provided Spectrum.
         *
         * @param owner
         */
        explicit PagingDevice128k(Spectrum128k & owner)
        : m_pagingEnabled(true),
          m_spectrum(owner)
        {}

        ~PagingDevice128k() override;
        
        /**
         * Check whether the paging device provides data on a specified IO port.
         *
         * Paging devices can be written to but not read from.
         *
         * @param port
         * @return false
         */
        [[nodiscard]] inline bool checkReadPort(::Z80::UnsignedWord port) const override
        {
            return false;
        }

        /**
         * Check whether the paging device is listening on a specified IO port.
         *
         * The paging device will respond to writes on any port with bits 2 and 15 set low.
         *
         * @param port
         * @return
         */
        [[nodiscard]] inline bool checkWritePort(::Z80::UnsignedWord port) const override
        {
            // respond if bits 2 and 15 are both set low in the port, other bits are irrelevant
            return (0b0111111111111101 == (port | 0b0111111111111101));
        }

        /**
         * Read a byte using a given port.
         *
         * Paging devices can be written to but not read from. This method always returns 0xff;
         *
         * @param port
         * @return
         */
        Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override
        {
            return 0xff;
        }

        /**
         * Write a byte to the paging device on the specified port.
         *
         * Note that this method assumes the Z80 has already called checkWritePort() to ensure that the device wants to
         * receive the data and therefore does not check the port.
         *
         * The value written is interpreted as:
         * bits 0-2: the RAM bank to page into the upper 16kb of the address space
         * bit 3:    which screen buffer to use for output (0 = normal buffer [bank 5], 1 = shadow buffer [bank 7])
         * bit 4:    which ROM to page into the lower 16kb of the address space
         * bit 5:    if set, disable all paging features from now until the Spectrum is reset
         *
         * @param port
         * @param value
         */
        void writeByte(::Z80::UnsignedWord port, ::Z80::UnsignedByte value) override;

        /**
         * Fetch the Spectrum 128K whose memory paging is being managed by this device.
         *
         * @return A const reference to the Spectrum.
         */
        [[nodiscard]] const Spectrum128k & spectrum() const
        {
            return m_spectrum;
        }

        /**
         * Fetch the Spectrum 128K whose memory paging is being managed by this device.
         *
         * @return A reference to the Spectrum.
         */
        Spectrum128k & spectrum()
        {
            return m_spectrum;
        }

        /**
         * Check whether paging is currently enabled.
         */
        [[nodiscard]] bool pagingEnabled() const
        {
            return m_pagingEnabled;
        }

        /**
         * Set whether paging should be enabled or not.
         *
         * This should only need to be called internally in response to the managed Spectrum being reset or when
         * applying a snapshot. Generally, it should not be used.
         *
         * @param enabled
         */
        void setPagingEnabled(bool enabled)
        {
            m_pagingEnabled = enabled;
        }

        /**
         * Reset the device.
         *
         * After reset, paging is enabled again.
         */
        void reset()
        {
            setPagingEnabled(true);
        }

    private:
        /**
         * Flag indicating whether memory paging is currently enabled.
         */
        bool m_pagingEnabled;

        /**
         * The Spectrum 128K whose memory paging is being managed.
         */
        Spectrum128k & m_spectrum;
    };
}

#endif //SPECTRUM_PAGINGDEVICE128K_H
