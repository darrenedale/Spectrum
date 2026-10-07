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

#include "pokefinder.h"

#include <ranges>

#include "../util/debug.h"

using namespace Spectrum;

void PokeFinder::setMemory(Memory * memory) noexcept
{
    m_memory = memory;
    searchForBeforeValue();
}

const PokeFinder::Addresses & PokeFinder::matchingBeforeAddresses() const noexcept
{
    return m_possibleAddresses;
}

PokeFinder::Addresses PokeFinder::matchingAfterAddresses(const std::optional<Word> newLives) const noexcept
{
    const Word actualNewLives = newLives.value_or(lives() - 1);
    Util::debugln("looking for {} in {} matching before addresses", actualNewLives, m_possibleAddresses.size());

    // TODO this algorithm is ripe for templating
    const auto possibleAddresses = std::ranges::views::filter(
        m_possibleAddresses,
        [actualNewLives, this](const Address address) -> bool {
            if (address < m_memory->addressableSize() - 1) {
                Util::debugln(
                    "Checking word {} at {:#06x} == {}",
                    m_memory->readWord<Word>(address),
                    address,
                    actualNewLives
                );
            }

            Util::debugln(
        "Checking byte {} at {:#06x} == {}",
                static_cast<std::uint16_t>(m_memory->readByte(address)),
                address,
                actualNewLives
            );

            return (
                // if the address permits a 16-bit read, compare the word value
                address < m_memory->addressableSize() - 1
                && actualNewLives == m_memory->readWord<Word>(address)
            )
            // compare the byte value
            || actualNewLives == m_memory->readByte(address);
        }
    ) | std::ranges::to<Addresses>();

    Util::debugln("found {} addresses that now have the value {}", possibleAddresses.size(), actualNewLives);
    return possibleAddresses;
}

void PokeFinder::searchForBeforeValue() noexcept
{
    m_possibleAddresses.clear();

    if (!hasLives()) {
        return;
    }

    Util::debugln("Looking for storage locations for {} lives", lives());

    for (Address address = 0x0000; address < m_memory->addressableSize() - 1; ++address) {
        if (0xff00 == address) {
            Util::debugln(
                "Address {:#06x} word: {}",
                address,
                m_memory->readWord<Word>(address)
            );

            Util::debugln(
                "Address {:#06x} byte: {}",
                address,
                static_cast<std::uint16_t>(m_memory->readByte(address))
            );
        }

        if (
            // compare both Word and Byte values
            lives() == m_memory->readWord<Word>(address)
            || lives() == static_cast<std::uint16_t>(m_memory->readByte(address))
        ) {
            m_possibleAddresses.emplace_back(static_cast<Address>(address));
        }
    }

    // doing the last byte separately keeps the loop simpler and marginally faster
    if (lives() == m_memory->readByte(m_memory->addressableSize() - 1)) {
        m_possibleAddresses.push_back(m_memory->addressableSize() - 1);
    }

    Util::debugln("Found {} memory locations with values matching lives {}", m_possibleAddresses.size(), lives());
}
