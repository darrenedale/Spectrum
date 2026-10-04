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
     * The poke finder currently only reads the addressable RAM currently in the provided memory. If it's a paging
     * memory, it currently won't map any pages, so whatever pages are not mapped at the point the poke finder scans are
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

            explicit PokeFinder(Memory * memory)
            : m_memory(memory),
              m_lives(0),
              m_score(0),
              m_originalBytes{}     // not initialised with any determinate value: copyMemory() will do that immediately
            {
                copyMemory();
            }

            /**
             * Set the original number of lives that the player has.
             *
             * Memory locations that change from this value to the value provided to possibleLivesAddresses() (or the
             * original lives less one if no value is provided) will be considered possible locations for the game's
             * lives.
             *
             * Until you set the lives, the PokeFinder won't scan for the lives value, so no pokes will be found.
             */
            void setLives(const Word lives) noexcept
            {
                m_lives = lives;
                searchForLives();
            }

            [[nodiscard]]
            Word lives() const noexcept
            {
                return m_lives;
            }

            /**
             * Set the original score that the player has.
             *
             * Memory locations that change from this value to the value provided to possibleScoreAddresses() will be
             * considered possible locations for the game's score.
             *
             * Until you set a score, the PokeFinder won't scan for the score value, so no pokes will be found.
             */
            void setScore(Word score) noexcept
            {
                m_score = score;
                searchForScore();
            }

            /** The original score. Only used when searching for a score poke is enabled. */
            [[nodiscard]]
            Word score() const noexcept
            {
                return m_score;
            }

            /**
             * The possible addresses where the lives might be located.
             *
             * @param newLives The optional number lives that the player now has. If omitted or an empty optional, the
             * original number of lives less one is used.
             */
            [[nodiscard]]
            Addresses possibleLivesAddresses(std::optional<Word> newLives = {}) const noexcept;

            /**
             * The possible addresses where the score might be located.
             *
             * @param newScore The new score that the player now has.
             */
            [[nodiscard]]
            Addresses possibleScoreAddresses(Word newScore) const noexcept;

        private:
            void copyMemory() noexcept;

            void searchForLives() noexcept;

            void searchForScore() noexcept;

            Memory * m_memory;
            Word m_lives;
            Word m_score;
            Addresses m_possibleLives;
            Addresses m_possibleScore;;
            std::array<Memory::Byte, 0x10000> m_originalBytes;
    };
}

#endif //SPECTRUM_POKEFINDER_H
