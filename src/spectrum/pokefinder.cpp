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

using namespace Spectrum;

PokeFinder::Addresses PokeFinder::possibleLivesAddresses(const std::optional<Word> newLives) const noexcept
{
    const Word actualNewLives = newLives.value_or(lives() - 1);

    // TODO this algorithm is ripe for templating
    return std::ranges::views::filter(
        m_possibleLives,
        [actualNewLives, this](const Address address) -> bool {
            return (
                // if the address permits a 16-bit read, compare the word value
                address < m_originalBytes.size() - 1
                && actualNewLives == ::Z80::z80ToHostByteOrder(*reinterpret_cast<const Word *>(m_originalBytes.data() + address))
            )
            // compare the byte value
            || actualNewLives == m_originalBytes[address];
        }
    ) | std::ranges::to<Addresses>();
}

PokeFinder::Addresses PokeFinder::possibleScoreAddresses(const Word newScore) const noexcept
{
    return std::ranges::views::filter(
        m_possibleLives,
        [newScore, this](const Address address) -> bool {
            return (
                // if the address permits a 16-bit read, compare the word value
                address < m_originalBytes.size() - 1
                && newScore == ::Z80::z80ToHostByteOrder(*reinterpret_cast<const Word *>(m_originalBytes.data() + address))
            )
            // compare the byte value
            || newScore == m_originalBytes[address];
        }
    ) | std::ranges::to<Addresses>();
}

void PokeFinder::copyMemory() noexcept
{
    m_memory->readBytes(0x0000, m_originalBytes.size(), m_originalBytes.data());
}

void PokeFinder::searchForLives() noexcept
{
    m_possibleLives.clear();

    for (Address address = 0x0000; address < m_originalBytes.size() - 1; address++) {
        if (
            // compare both Word and Byte values
            lives() == ::Z80::z80ToHostByteOrder(*reinterpret_cast<const Word *>(m_originalBytes.data() + address))
            || lives() == m_originalBytes[address]
        ) {
            m_possibleLives.emplace_back(static_cast<Address>(address));
        }
    }

    // doing this separately keeps the loop simpler and marginally faster
    if (lives() == *m_originalBytes.cend()) {
        m_possibleLives.push_back(m_originalBytes.size() - 1);
    }
}

void PokeFinder::searchForScore() noexcept
{
    m_possibleScore.clear();

    for (Address address = 0x0000; address < m_originalBytes.size() - 1; address++) {
        if (
            // compare both Word and Byte values
            score() == ::Z80::z80ToHostByteOrder(*reinterpret_cast<const Word *>(m_originalBytes.data() + address))
            || score() == m_originalBytes[address]
        ) {
            m_possibleScore.emplace_back(static_cast<Address>(address));
        }
    }

    // doing this separately keeps the loop simpler and marginally faster
    if (score() == *m_originalBytes.cend()) {
        m_possibleScore.push_back(m_originalBytes.size() - 1);
    }
}
