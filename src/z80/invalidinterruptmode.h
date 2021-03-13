//
// Created by darren on 12/03/2021.
//

#ifndef Z80_INVALIDINTERRUPTMODE_H
#define Z80_INVALIDINTERRUPTMODE_H

namespace Z80
{
    class InvalidInterruptMode
    : public std::exception
    {
    public:
        explicit InvalidInterruptMode(UnsignedByte mode)
        : m_mode(mode)
        {}

        InvalidInterruptMode(const InvalidInterruptMode & other) = default;
        InvalidInterruptMode(InvalidInterruptMode && other) = default;
        InvalidInterruptMode & operator=(const InvalidInterruptMode & other) = default;
        InvalidInterruptMode & operator=(InvalidInterruptMode && other) = default;
        ~InvalidInterruptMode() override = default;

        [[nodiscard]] UnsignedByte mode() const
        {
            return m_mode;
        }

    private:
        UnsignedByte m_mode;
    };
}

#endif //Z80_INVALIDINTERRUPTMODE_H
