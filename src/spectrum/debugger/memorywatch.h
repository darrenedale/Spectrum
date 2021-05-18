//
// Created by darren on 01/05/2021.
//

#ifndef SPECTRUM_DEBUGGER_MEMORYWATCH_H
#define SPECTRUM_DEBUGGER_MEMORYWATCH_H

#include "../../z80/types.h"
#include "../basespectrum.h"

namespace Spectrum::Debugger
{
    /**
     * Abstract base class for debug watches on Computer memory.
     */
    class MemoryWatch
    {
    public:
        /**
         * Convenience alias for the size, in bytes, of a chunk of watched memory.
         */
        using WatchSize = std::uint32_t;

        /**
         * Initialise a new watch for a given address in a given memory object.
         *
         * The memory must not be null and must be fully addressable for all the bytes watched.
         *
         * @param memory The memory object to watch.
         * @param address The address to watch.
         */
        MemoryWatch(BaseSpectrum::MemoryType * memory, ::Z80::UnsignedWord address)
        : m_memory(memory),
          m_address(address)
        {
            assert(memory);
            assert(address < memory->addressableSize());
        }

        /**
         * Destructor.
         */
        virtual ~MemoryWatch() = default;

        /**
         * Fetch the memory that is being watched.
         *
         * @return The memory.
         */
        [[nodiscard]] BaseSpectrum::MemoryType * memory() const
        {
            return m_memory;
        }

        /**
         * Set the address that is being watched.
         *
         * The memory must not be null and must support addressing all the bytes being watched.
         *
         * @param memory The memory.
         */
        void setMemory(BaseSpectrum::MemoryType * memory)
        {
            assert(memory);
            assert(address() < memory->addressableSize());
            m_memory = memory;
        }

        /**
         * Fetch the address that is being watched.
         *
         * @return The address.
         */
        [[nodiscard]] ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

        /**
         * Set the address that is being watched.
         *
         * The memory must support addressing all the bytes being watched from the new address.
         *
         * @param address The address.
         */
        void setAddress(::Z80::UnsignedWord address)
        {
            assert(address < memory()->addressableSize());
            m_address = address;
        }

        /**
         * The size in bytes of the block of memory that the watch is observing.
         *
         * @return The number of bytes, starting at address().
         */
        virtual WatchSize size() const = 0;

        /**
         * The label for the watch.
         *
         * The label will be an empty string if none has been set.
         *
         * @return The label.
         */
        [[nodiscard]] const std::string & label() const
        {
            return m_label;
        }

        /**
         * Set the label for the watch.
         *
         * @param label
         */
        void setLabel(std::string label)
        {
            m_label = std::move(label);
        }

        /**
         * Provides a user-visible name for the type of watch.
         *
         * Watches of a given type should return fundamentally the same type name. It is recommended that each watch type has a common type name (e.g. "String")
         * optionally followed by the size of the watch. For example:
         * - "String [10]" A string of 10 bytes
         * - "String [20]" A string of 20 bytes
         * - "Int [8]" An 8-bit integer
         * - "Int [16]" A 16-bit integer
         * - "Int [32]" A 32-bit integer
         * - "Int [64]" A 64-bit integer
         *
         * @return
         */
        virtual std::string typeName() const = 0;

        /**
         * Fetch the content to display for the current value of the watched address.
         *
         * @return The display content.
         */
        virtual std::string displayValue() const = 0;

    private:
        /**
         * The memory being watched.
         */
        BaseSpectrum::MemoryType * m_memory;

        /**
         * The address of the first byte of memory being watched.
         */
        ::Z80::UnsignedWord m_address;

        /**
         * The arbitrary label to give to the watch.
         */
        std::string m_label;
    };
}

#endif //SPECTRUM_DEBUGGER_MEMORYWATCH_H
