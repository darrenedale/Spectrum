//
// Created by darren on 23/04/2021.
//

#ifndef SPECTRUM_MAPPABLEMEMORYINTERFACE_H
#define SPECTRUM_MAPPABLEMEMORYINTERFACE_H

template <class byte_t = std::uint8_t, class address_t = std::uint64_t, class size_t = std::uint64_t>
class MappableMemoryInterface
{
    static_assert(std::is_integral_v<byte_t>, "byte type for memory must be an integer type");
    static_assert(std::is_integral_v<address_t>, "address type for memory must be an integer type");
    static_assert(std::is_integral_v<size_t>, "size type for memory must be an integer type");

public:
    virtual void mapMemory(address_t startAddress, byte_t * storage, size_t size) = 0;
    virtual void unmapMemory(address_t startAddress, const byte_t * storage) = 0;
};

#endif //SPECTRUM_MAPPABLEMEMORYINTERFACE_H
