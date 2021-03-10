//
// Created by darren on 10/03/2021.
//

#ifndef Z80_INVALIDOPCODE_H
#define Z80_INVALIDOPCODE_H

#include <vector>

namespace Z80
{
    class InvalidOpcode
    : public std::exception
    {
    public:
        explicit InvalidOpcode(std::vector<UnsignedByte> opcode, UnsignedWord location = 0)
        : m_opcode(std::move(opcode)),
          m_location(location)
        {}

        InvalidOpcode(const InvalidOpcode & other) = default;
        InvalidOpcode(InvalidOpcode && other) = default;
        InvalidOpcode & operator=(const InvalidOpcode & other) = default;
        InvalidOpcode & operator=(InvalidOpcode && other) = default;
        ~InvalidOpcode() override = default;

        [[nodiscard]] UnsignedWord location() const
        {
            return m_location;
        }

        [[nodiscard]] int size() const
        {
            return static_cast<int>(m_opcode.size());
        }

    private:
        std::vector<UnsignedByte> m_opcode;
        UnsignedWord m_location;
    };
}

#endif //Z80_INVALIDOPCODE_H
