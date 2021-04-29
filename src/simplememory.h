//
// Created by darren on 25/03/2021.
//

#ifndef SIMPLEMEMORY_H
#define SIMPLEMEMORY_H

#include "memory.h"
#include <cstring>
#include <vector>

/**
 * Simple memory for a computer - an array of bytes.
 *
 * This represents a simple linear address space that is fully allocated and does not do any paging or mapping.
 *
 * @tparam byte_t The storage type for a single byte of memory. Defaults to std::uint8_t (an unsigned 8-bit value).
 * @tparam address_t The storage type to represent an address. Defaults to std::uint64_t (an unsigned 64-bit value).
 * @tparam size_t The storage type for the memory size. Defaults to std::uint64_t (an unsigned 64-bit value).
 */
template<class byte_t = std::uint8_t, class address_t = std::uint64_t, class size_t = std::uint64_t>
class SimpleMemory
: public Memory<byte_t, address_t, size_t>
{
public:
    using Address = std::uint64_t;
    using Size = std::uint64_t;
    using Byte = byte_t;

    explicit SimpleMemory(Size addressableSize = 0, std::optional<Size> availableSize = {})
    : Memory<byte_t, address_t, size_t>(addressableSize, std::move(availableSize)),
      m_storage(this->availableSize(), 0)
    {}

    SimpleMemory(const Memory<byte_t, address_t, size_t> &) = delete;
    SimpleMemory(Memory<byte_t, address_t, size_t> &&) = delete;
    void operator=(const Memory<byte_t, address_t, size_t> &) = delete;
    void operator=(Memory<byte_t, address_t, size_t> &&) = delete;
    ~SimpleMemory() override = default;

    /**
     * Set all installed memory to 0 bytes.
     */
    void clear() override
    {
        std::memset(m_storage.data(), 0, this->availableSize());
    }

    std::unique_ptr<Memory<byte_t, address_t, size_t>> clone() const override
    {
        auto ret = std::make_unique<SimpleMemory<byte_t, address_t, size_t>>(this->addressableSize(), this->availableSize());
        std::memcpy(ret->m_storage.data(), this->m_storage.data(), this->availableSize());
        return ret;
    }

protected:
    /**
     * Helper to map a virtual memory address to the actual location of the requested byte in host storage.
     *
     * In the default implementation this is simply the offset of the address from the start of the linear storage.
     * However, other memory models will work differently (e.g. 128k Spectrums page banks of memory in and out of the
     * Z80 address space.
     *
     * @param address
     * @return
     */
    virtual inline Byte * mapAddress(Address address) const override
    {
        return const_cast<Byte *>(m_storage.data()) + address;
    }

private:
    std::vector<Byte> m_storage;
};

#endif //SIMPLEMEMORY_H
