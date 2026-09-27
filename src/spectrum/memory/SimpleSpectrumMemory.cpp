//
// Created by darren on 23/04/2021.
//

#include <algorithm>
#include <cstdio>
#include <print>
#include "SimpleSpectrumMemory.h"
#include "../../util/assert.h"

namespace Spectrum::Memory
{
    using BaseMemory = SimpleMemory<::Z80::UnsignedByte>;

    SimpleSpectrumMemory::SimpleSpectrumMemory(SimpleMemory::Size availableSize)
    : SimpleMemory<>(0x10000, availableSize),    // all Spectrum memory has 64k addressable size
      m_mappedMemory()
    {}

    SimpleSpectrumMemory::~SimpleSpectrumMemory() = default;

    void SimpleSpectrumMemory::mapMemory(Address startAddress, unsigned char * storage, SimpleMemory::Size size)
    {
        sp_assert(storage, "null storage provided to {}", __FUNCTION__);
        sp_assert(startAddress < addressableSize(), "start address {} overflows the addressable range {}", startAddress, addressableSize());
        sp_assert(startAddress + size <= addressableSize(), "size {} overflows the addressable range {}", size, addressableSize());

        m_mappedMemory.emplace_back(MappedMemoryBlock{
            .address = startAddress,
            .size = size,
            .storage = storage,
        });
    }

    void SimpleSpectrumMemory::unmapMemory(Address startAddress, const Byte * storage)
    {
        const auto pos = std::find_if(m_mappedMemory.crbegin(), m_mappedMemory.crend(), [startAddress, storage](const MappedMemoryBlock & block) -> bool {
            return block.address == startAddress && block.storage == storage;
        });

        sp_assert(pos != m_mappedMemory.crend(), "memory block starting at {} not found in storage", startAddress);
        m_mappedMemory.erase(pos.base());
    }

    bool SimpleSpectrumMemory::isMapped(Address startAddress, const Byte * storage)
    {
        return storage && startAddress < addressableSize() && m_mappedMemory.cend() != std::find_if(m_mappedMemory.cbegin(), m_mappedMemory.cend(), [startAddress, storage](const MappedMemoryBlock & block) -> bool {
            return block.address == startAddress && block.storage == storage;
        });
    }

    unsigned char * SimpleSpectrumMemory::mapAddress(Address startAddress) const
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
