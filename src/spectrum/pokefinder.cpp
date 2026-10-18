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

#include <chrono>
#include <ranges>

#include "../util/debug.h"

using namespace Spectrum;

namespace
{
    constexpr int InitialMatchingAddressCapacity = 2048;
}

PokeFinder::PokeFinder(Memory* memory)
: m_memory(memory)
{
    m_possibleAddresses.reserve(InitialMatchingAddressCapacity);
}

void PokeFinder::setMemory(Memory * memory) noexcept
{
    m_memory = memory;
    searchForBeforeValue();
}

const PokeFinder::Addresses & PokeFinder::matchingBeforeAddresses() const noexcept
{
    return m_possibleAddresses;
}

PokeFinder::Addresses PokeFinder::matchingAfterAddresses(const std::optional<Word> newValue) const noexcept
{
    const Word actualNewValue = newValue.value_or(before() - 1);
    Util::debugln("looking for {} in {} matching before addresses", actualNewValue, m_possibleAddresses.size());

    const auto possibleAddresses = std::ranges::views::filter(
        m_possibleAddresses,
        [actualNewValue, this](const Address address) -> bool {
            return (
                // if the address permits a 16-bit read, compare the word value
                address < m_memory->addressableSize() - 1
                && actualNewValue == m_memory->readWord<Word>(address)
            )
            // compare the byte value
            || actualNewValue == m_memory->readByte(address);
        }
    ) | std::ranges::to<Addresses>();

    Util::debugln("found {} addresses that now have the value {}", possibleAddresses.size(), actualNewValue);
    return possibleAddresses;
}

void PokeFinder::searchForBeforeValue() noexcept
{
    m_possibleAddresses.clear();

    if (!hasBefore()) {
        return;
    }

    Util::debugln("looking for addresses containing {}", before());
    const auto value = before();

#if !defined(NDEBUG)
    const auto start = std::chrono::steady_clock::now();
#endif

    for (Address address = 0x0000; address < m_memory->addressableSize() - 1; ++address) {
        if (
            // compare both Word and Byte values
            value == m_memory->readWord<Word>(address)
            || value == static_cast<::Z80::UnsignedWord>(m_memory->readByte(address))
        ) {
            m_possibleAddresses.push_back(address);
        }
    }

    // doing the last byte separately keeps the loop simpler and marginally faster
    if (value == m_memory->readByte(m_memory->addressableSize() - 1)) {
        m_possibleAddresses.push_back(m_memory->addressableSize() - 1);
    }

#if !defined(NDEBUG)
    const auto duration = std::chrono::steady_clock::now() - start;

    Util::debugln(
        "Found {} addresses with value {} in {} ms",
        m_possibleAddresses.size(),
        value,
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
    );
#endif
}
