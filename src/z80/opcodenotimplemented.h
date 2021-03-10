//
// Created by darren on 10/03/2021.
//

#ifndef Z80_OPCODENOTIMPLEMENTED_H
#define Z80_OPCODENOTIMPLEMENTED_H

#include <vector>

namespace Z80
{
    class OpcodeNotImplemented
    : public std::exception
    {
    public:
        explicit OpcodeNotImplemented(std::vector<UnsignedByte> opcode, UnsignedWord location = 0)
        : OpcodeNotImplemented(std::move(opcode), {}, location)
        {}

        OpcodeNotImplemented(std::vector<UnsignedByte> opcode, std::string message, UnsignedWord location = 0)
        : std::exception(),
          m_opcode(std::move(opcode)),
          m_location(location),
          m_message(std::move(message))
        {}

        OpcodeNotImplemented(const OpcodeNotImplemented & other) = default;
        OpcodeNotImplemented(OpcodeNotImplemented && other) = default;
        OpcodeNotImplemented & operator=(const OpcodeNotImplemented & other) = default;
        OpcodeNotImplemented & operator=(OpcodeNotImplemented && other) = default;
        ~OpcodeNotImplemented() override = default;

        [[nodiscard]] UnsignedWord location() const
        {
            return m_location;
        }

        [[nodiscard]] int size() const
        {
            return static_cast<int>(m_opcode.size());
        }

        [[nodiscard]] const std::string & message() const
        {
            return m_message;
        }

    private:
        std::vector<UnsignedByte> m_opcode;
        UnsignedWord m_location;
        std::string m_message;
    };
}
#endif //Z80_OPCODENOTIMPLEMENTED_H
