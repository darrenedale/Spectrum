#ifndef Z80IODEVICE_H
#define Z80IODEVICE_H

#include "z80.h"

namespace Z80 {
    class Z80IODevice {
        friend class Z80;

    public:
        Z80IODevice();
        virtual ~Z80IODevice();

        /**
         * Ask the device if it's interested in providing input on a given port.
         *
         * @param port
         * @return
         */
        virtual bool checkReadPort(Z80::UnsignedWord port) const = 0;

        /**
         * Ask the device if it's interested in receiving output on a given port.
         *
         * @param port
         * @return
         */
        virtual bool checkWritePort(Z80::UnsignedWord port) const = 0;

        /**
         * Read a byte from the device using the given port.
         *
         * @param port
         * @return
         */
        virtual Z80::UnsignedByte readByte(Z80::UnsignedWord port) = 0;

        /**
         * Write a byte to the device using the given port.
         *
         * @param port
         * @return
         */
        virtual void writeByte(Z80::UnsignedWord port, Z80::UnsignedByte b) = 0;

    protected:
        void setCpu(Z80 *cpu)
        {
            m_cpu = cpu;
        }

        Z80 * cpu()
        {
            return m_cpu;
        }

    private:
        Z80 * m_cpu;
    };
}

#endif // Z80IODEVICE_H
