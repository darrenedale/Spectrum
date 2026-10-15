/*
 * Copyright (c) 2026 Darren Edale
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SPECTRUM_POKEFINDER_H
#define SPECTRUM_POKEFINDER_H

#include "basespectrum.h"
#include "../z80/z80.h"
#include "../z80/types.h"
#include "devices/cursorjoystick.h"
#include "devices/displaydevice.h"

namespace Spectrum
{
    /**
     * The poke finder helps players find the addresses of game variables. The basic idea is this:
     * - the player provides the current value of the game variable (e.g. their lives) to setBeforer()
     * - the poke finder scans the Spectrum memory for addresses where that value is found
     * - the player plays the game until the variable's value changes
     * - the player provides the new value of the game variable to matchingAfterAddresses()
     * - the poke finder scans the previously identified addresses for the new value, and returns all those that match
     *
     * The poke finder currently only reads the addressable RAM in the provided memory. If it's a paging memory, it
     * currently won't map any pages, so whatever pages are not mapped at the point the poke finder scans are
     * not scanned. It therefore assumes that the same pages are mapped when the memory is provided and when it's
     * scanned for possible poke locations.
     *
     * This sort of makes sense anyway, since, when the Spectrum is running, you can only poke to the mapped pages. It
     * could be possible to poke directly into other pages, which may be considered in future.
     */
    class PokeFinder
    {
        public:
            using Memory = BaseSpectrum::MemoryType;
            using Address = Memory::Address;
            using Addresses = std::vector<Address>;
            using Byte = ::Z80::UnsignedByte;
            using Word = ::Z80::UnsignedWord;

            explicit PokeFinder(Memory * memory);

            // poke finders don't need to be copied or moved
            PokeFinder(const PokeFinder &) = delete;
            PokeFinder(PokeFinder &&) = delete;
            PokeFinder operator=(const PokeFinder &) = delete;
            PokeFinder operator=(PokeFinder &&) = delete;
            ~PokeFinder() = default;

            /**
             * Set the memory that the poke finder will scan. This is borrowed, not owned. The caller is responsible
             * for ensuring its lifetime matches or exceeds that of the poke finder.
             */
            void setMemory(Memory *) noexcept;

            /**
             * Set the original value to search for.
             *
             * Memory locations that change from this value to the value provided to matchingAfterAddresses() (or the
             * original value less one if no value is provided) will be considered possible locations for the sought
             * game variable (typically lives).
             *
             * Until you set the value, the PokeFinder won't scan, so no pokes will be found.
             */
            void setBefore(const Word value) noexcept
            {
                m_beforeValue = value;
                searchForBeforeValue();
            }

            /** Whether the value to scan for has been set. */
            [[nodiscard]]
            bool hasBefore() const noexcept
            {
                return m_beforeValue.has_value();
            }

            /**
             * The original ("before") value.
             *
             * It's a programmer error to call this if no before value has been set. Call hasBefore() first if it's
             * possible the value has not been set.
             */
            [[nodiscard]]
            Word before() const noexcept
            {
                return m_beforeValue.value();
            }

            /** The addresses where the before value is located. */
            [[nodiscard]]
            const Addresses & matchingBeforeAddresses() const noexcept;

            /**
             * The possible addresses where the sought game variable might be located.
             *
             * @param newValue The optional value that the game variable now has. If omitted or an empty optional, the
             * original value less one is used (on the assumption that the player is looking for the lives variable, and
             * has just lost one).
             */
            [[nodiscard]]
            Addresses matchingAfterAddresses(std::optional<Word> newValue = {}) const noexcept;

        private:
            /** Helper that does the initial memory scan. */
            void searchForBeforeValue() noexcept;

            /** The memory to scan. This is borrowed, not owned. */
            Memory * m_memory;

            /** The "before" value, if set. */
            std::optional<Word> m_beforeValue;

            /**
             * The list of addresses that match the "before" value, if it's set.
             *
             * This is empty if the "before" value is not set.
             */
            Addresses m_possibleAddresses;
    };
}

#endif //SPECTRUM_POKEFINDER_H
