#ifndef Z80IODEVICE_H
#define Z80IODEVICE_H

#include "z80.h"

namespace Z80 {
    class Z80IODevice {
        friend class Z80;

    public:
        Z80IODevice();

        virtual ~Z80IODevice();

        virtual Z80::UnsignedByte readByte() = 0;

        virtual void writeByte(Z80::UnsignedByte b) = 0;

    protected:
        void setCpu(Z80 *cpu) {
            m_cpu = cpu;
        }

    private:
        Z80 * m_cpu;
    };
}

#endif // Z80IODEVICE_H
