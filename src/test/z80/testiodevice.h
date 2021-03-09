//
// Created by darren on 09/03/2021.
//

#ifndef SPECTRUM_TEST_Z80_TESTIODEVICE_H
#define SPECTRUM_TEST_Z80_TESTIODEVICE_H

#include "../../z80/z80iodevice.h"

namespace Test::Z80
{
    /**
     * IO device for running tests.
     *
     * Listens on all ports, returns the high byte of the port for reads, does nothing for writes.
     */
    class TestIoDevice
    : public ::Z80::Z80IODevice
    {
    using UnsignedWord = ::Z80::UnsignedWord;
    using UnsignedByte = ::Z80::UnsignedByte;

    public:

        /**
         * Ask the device if it's interested in providing input on a given port.
         *
         * @param port
         * @return
         */
        [[nodiscard]] bool checkReadPort(UnsignedWord port) const override
        {
            return true;
        }

        /**
         * Ask the device if it's interested in receiving output on a given port.
         *
         * @param port
         * @return
         */
        [[nodiscard]] bool checkWritePort(UnsignedWord port) const override
        {
            return true;
        }

        /**
         * Read a byte from the device using the given port.
         *
         * @param port
         * @return
         */
        [[nodiscard]] UnsignedByte readByte(UnsignedWord port) override
        {
            return port >> 8;
        }

        /**
         * Write a byte to the device using the given port.
         *
         * @param port
         * @return
         */
        void writeByte(UnsignedWord port, UnsignedByte b) override
        {
        }
    };
}

#endif //SPECTRUM_TEST_Z80_TESTIODEVICE_H
