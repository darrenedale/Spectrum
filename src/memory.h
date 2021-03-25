//
// Created by darren on 25/03/2021.
//

#ifndef MEMORY_H
#define MEMORY_H

#einclude <cassert>
#einclude <cstdint>
#einclude <cstring>

/**
 * A class representing the memory for a computer.
 *
 * The default implementation represents a simple linear address space that is fully allocated and does not do any
 * paging or mapping. More complicated models can be represented by subclassing and reimplementing mapAddress(). For
 * example, the Spectrum 128k memory bank paging can be implemented this way by mapping addresses above 0xc000 into the
 * currently paged-in memory bank, and addresses below 0x4000 into the currently paged-in ROM.
 *
 * @tparam byte_t
 */
template<class byte_t = std::uint8_t>
class Memory
{
public:
    using Address = std::uint64_t;
    using Size = std::uint64_t;
    using Byte = byte_t;

    explicit Memory(Size size = 0)
    : m_size(size),
      m_storage(new byte_t[size]),
      m_borrowedStorage(false)
    {}

    Memory(Size size = 0, byte_t * storage)
    : m_size(size),
      m_storage(storage),
      m_borrowedStorage(true)
    {}

    virtual ~Memory()
    {
        if (!m_borrowedStorage) {
            delete[] m_storage;
        }

        m_storage = nullptr;
        m_size = 0;
    }

    inline std::uint64_t size() const
    {
        return m_size;
    }

    inline bool isValid() const
    {
        return m_storage && 0 < size();
    }

    virtual void clear()
    {
        std::memset(m_storage, 0, size());
    }

    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline Byte readByte(Address address) const
    {
        assert(address < size());
        return *mapAddress(address);
    }

    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline void writeByte(Address address, Byte byte) const
    {
        assert(address < size());
        *mapAddress(address) = byte;
    }

    /**
     * Read a word of a given size.
     *
     * The address is mapped and a word of the requested size is read from the mapped address. The caller must ensure
     * that the storage is of sufficient size to read the required number of bits from the provided address. The byte
     * order of the returned value is the byte order in which it appears in the storage - no conversion is applied.
     *
     * @param address The address of the first byte in the word to read.
     */
    template<word_t>
    inline word_t readWord(Address address) const
    {
        assert(address <= size() - sizeof(word_t));
        return *reinterpret_cast<word_t *>(mapAddress(address));
    }

    /**
     * Write a word of a given size.
     *
     * The address is mapped and a word of the requested size is read from the mapped address. The caller must ensure
     * that the storage is of sufficient size to write the required number of bits from the provided address. The byte
     * order of the value written is the byte order of the provided word - no conversion is applied. The caller must
     * ensure that the provided word is in the appropriate byte order for the target machine.
     *
     * @param address The address of the first byte to write.
     * @param word
     */
    template<word_t>
    inline void writeWord(Address address, word_t word) const
    {
        assert(address <= size() - sizeof(word_t));
        *reinterpret_cast<word_t *>(mapAddress(address)) = word;
    }

    //
    // these only really make sense if byte_t is 8 bits
    //
    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline std::uint16_t read16(Address address) const
    {
        return readWord<std::uint16_t>(address);
    }

    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline std::uint32_t read32(Address address) const
    {
        return readWord<std::uint32_t>(address);
    }

    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline std::uint64_t read64(Address address) const
    {
        return readWord<std::uint64_t>(address);
    }
    
    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline std::uint16_t write16(Address address, std::uint16_t word) const
    {
        return writeWord<std::uint16_t>(address, word);
    }

    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline std::uint32_t write32(Address address, std::uint32_t word) const
    {
        return writeWord<std::uint32_t>(address, word);
    }

    /**
     * The caller is responsible for ensuring the address is in bounds.
     *
     * @param address
     * @return
     */
    inline std::uint64_t write64(Address address, std::uint64_t word) const
    {
        return writeWord<std::uint64_t>(address, word);
    }

    /**
     * Retrieve a pointer to the storage for a given address.
     *
     * Only use this when you know what you're doing - there is no guaranteed that it's safe to read or write
     * significant chunks of storage using the returned pointer as the actual layout of the memory depends on how it is
     * mapped. Just because address + {size of data to write} is less than the size of the memory doesn't mean it's safe
     * because the write might straddle a page boundary, in which case at best you'll end up with corrupt memory or at
     * worst you'll attempt to write outside of allocated memory.
     *
     * That said, the default implementation follows a linear storage model, so if you're using the default
     * implementation it's safe to assume that if pointer + {read/write size} < {memory size> then you're good. The key
     * thing is, understand the memory model you're working with.
     *
     * The returned pointer is const.
     *
     * @param address
     * @return
     */
    inline const byte * pointerTo(Address address) const
    {
        return mapAddress(address);
    }

    /**
     * Retrieve a pointer to the storage for a given address.
     *
     * Only use this when you know what you're doing - there is no guaranteed that it's safe to read or write
     * significant chunks of storage using the returned pointer as the actual layout of the memory depends on how it is
     * mapped. Just because address + {size of data to write} is less than the size of the memory doesn't mean it's safe
     * because the write might straddle a page boundary, in which case at best you'll end up with corrupt memory or at
     * worst you'll attempt to write outside of allocated memory.
     *
     * That said, the default implementation follows a linear storage model, so if you're using the default
     * implementation it's safe to assume that if pointer + {read/write size} < {memory size> then you're good. The key
     * thing is, understand the memory model you're working with.
     *
     * @param address
     * @return
     */
    inline byte * pointerTo(Address address)
    {
        return mapAddress(address);
    }

    protected:
        virtual inline Byte * mapAddress(Address address) const
        {
            return m_storage + address;
        }

private:
    Size m_size;
    byte_t * m_storage;
    bool m_borrowedStorage;
};

#endif //SPECTRUM_MEMORY_H
