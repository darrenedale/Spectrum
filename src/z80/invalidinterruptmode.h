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

        InvalidOpcode(const InvalidOpcode & other) = default;
        InvalidOpcode(InvalidOpcode && other) = default;
        InvalidOpcode & operator=(const InvalidOpcode & other) = default;
        InvalidOpcode & operator=(InvalidOpcode && other) = default;
        ~InvalidOpcode() override = default;

        [[nodiscard]] UnsignedByte mode() const
        {
            return m_mode;
        }

    private:
        UnsignedByte m_mode;
    };
}

#endif //Z80_INVALIDINTERRUPTMODE_H
