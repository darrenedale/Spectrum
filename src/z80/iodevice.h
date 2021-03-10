#ifndef Z80_Z80IODEVICE_H
#define Z80_Z80IODEVICE_H

#include "z80.h"
#include "types.h"

namespace Z80
{
    class IODevice
    {
    friend class Z80;

    public:
        IODevice();
        virtual ~IODevice();

        /**
         * Ask the device if it's interested in providing input on a given port.
         *
         * @param port
         * @return
         */
        virtual bool checkReadPort(UnsignedWord port) const = 0;

        /**
         * Ask the device if it's interested in receiving output on a given port.
         *
         * @param port
         * @return
         */
        virtual bool checkWritePort(UnsignedWord port) const = 0;

        /**
         * Read a byte from the device using the given port.
         *
         * @param port
         * @return
         */
        virtual UnsignedByte readByte(UnsignedWord port) = 0;

        /**
         * Write a byte to the device using the given port.
         *
         * @param port
         * @return
         */
        virtual void writeByte(UnsignedWord port, UnsignedByte b) = 0;

    protected:
        void setCpu(Z80 * cpu)
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

#endif // Z80_Z80IODEVICE_H
