//
// Created by darren on 23/04/2021.
//

#ifndef SPECTRUM_MEMORY_H
#define SPECTRUM_MEMORY_H

#include "../simplememory.h"
#include "../mappablememoryinterface.h"
#include "../z80/types.h"

namespace Spectrum
{
    /**
     * Memory class for 16k and 48k Spectrums.
     *
     * These use a simple linear model of 16-bit addressable memory, only half of which is populated for the 16k model. The MappableMemoryInterface is added to
     * the SimpleMemory base to enable, for example, the ZX Interface 1 ROM to be mapped into the address space.
     */
    class Memory
    : public SimpleMemory<::Z80::UnsignedByte>,
      public MappableMemoryInterface<::Z80::UnsignedByte>
    {
    public:
        /**
         * Initialise a new Spectrum Memory object with a given available size.
         *
         * The addressable size of the memory is always 64k, as dictated by the 16-bit address bus of the Z80. The availableSize argument determines how much of
         * that address space is usable. The first usable address is always 0x0000.
         *
         * @param availableSize The number of available bytes in the memory.
         */
        explicit Memory(Size availableSize = 0x10000);

        /**
         * Destructor.
         */
        ~Memory() override;

        /**
         * Map an arbitrary block of memory into the address space starting at a given address.
         *
         * It is legitimate to map the same block to several different addresses, map different blocks to the same address, map overlapping blocks or even to
         * map the same block to the same address multiple times. Conflicts between mappings are resolved by assuming the latest mapping takes precedence. If
         * a block is mapped to the same address multiple times it must be unmapped the same number of times as it is mapped to become fully unmapped.
         *
         * The fist byte of the block of storage will appear at address, the second byte at address + 1 and so on up to address + size - 1.
         *
         * @param startAddress Where to map the storage. It must be within the addressable range of the memory.
         * @param storage The block of storage to map. It must not be nullptr, and must contain at least size bytes.
         * @param size The size of the block to map. It must not extend beyond the addressable range of the memory.
         */
        void mapMemory(Address startAddress, Byte * storage, Size size) override;

        /**
         * Unmap a block of memory from a given address.
         *
         * If a block has been mapped to the same address multiple times, unmapping will remove
         * the most recent mapping of that block to the address. Previous mappings of the block to the same address will remain in place. If the block is mapped
         * to other addresses, these mappings will also remain in place, as will any other blocks that are currently mapped to the same address.
         *
         * It is an error to attempt to unmap a block that is not mapped. (It follows that it is also an error to attempt to unmap a nullptr block.)
         * Check isMapped() if your code is not sure.
         *
         * @param startAddress The address of the mapping to remove. It must be within the addressable range of the memory.
         * @param storage The block of storage to unmap.
         */
        void unmapMemory(Address startAddress, const Byte * storage) override;

        /**
         * Check whether a given block of memory is mapped into a given address.
         *
         * @param startAddress The address to check.
         * @param storage The block of storage to check.
         *
         * @return
         */
        bool isMapped(Address startAddress, const Byte * storage);

    protected:
        [[nodiscard]] Byte * mapAddress(Address startAddress) const override;

    private:
        struct MappedMemoryBlock {
            Address address;
            Size size;
            Byte * storage;
        };

        using MappedMemory = std::vector<MappedMemoryBlock>;

        MappedMemory m_mappedMemory;
    };
}

#endif //SPECTRUM_MEMORY_H
