//
// Created by darren on 23/04/2021.
//

#include <algorithm>
#include <cassert>
#include "memory.h"

namespace Spectrum
{
    using BaseMemory = SimpleMemory<::Z80::UnsignedByte>;

    Memory::Memory(SimpleMemory::Size availableSize)
    : SimpleMemory<>(0x10000, availableSize),    // all Spectrum memory has 64k addressable size
      m_mappedMemory()
    {}

    Memory::~Memory() = default;

    void Memory::mapMemory(Address startAddress, unsigned char * storage, SimpleMemory::Size size)
    {
        assert(storage);
        assert(startAddress < addressableSize());
        assert(startAddress + size <= addressableSize());

        m_mappedMemory.emplace_back(MappedMemoryBlock{
            .address = startAddress,
            .size = size,
            .storage = storage,
        });
    }

    void Memory::unmapMemory(Address startAddress, const Byte * storage)
    {
        const auto pos =std::find_if(m_mappedMemory.crbegin(), m_mappedMemory.crend(), [startAddress, storage](const MappedMemoryBlock & block) -> bool {
            return block.address == startAddress && block.storage == storage;
        });

        assert(pos != m_mappedMemory.crend());
        m_mappedMemory.erase(pos.base());
    }

    bool Memory::isMapped(Address startAddress, const Byte * storage)
    {
        return storage && startAddress < addressableSize() && m_mappedMemory.cend() != std::find_if(m_mappedMemory.cbegin(), m_mappedMemory.cend(), [startAddress, storage](const MappedMemoryBlock & block) -> bool {
            return block.address == startAddress && block.storage == storage;
        });
    }

    unsigned char * Memory::mapAddress(Address startAddress) const
    {
        // check if the requested address is in a mapped memory block
        if (!m_mappedMemory.empty()) {
            // search mapped blocks in reverse - most recently mapped blocks take precedence
            const auto pos = std::find_if(m_mappedMemory.crbegin(), m_mappedMemory.crend(), [startAddress](const MappedMemoryBlock & block) -> bool {
                return block.address <= startAddress && block.address + block.size > startAddress;
            });

            if (pos != m_mappedMemory.crend()) {
                return pos->storage + startAddress - pos->address;
            }
        }

        // if not, delegate to the base class
        return BaseMemory::mapAddress(startAddress);
    }

}
