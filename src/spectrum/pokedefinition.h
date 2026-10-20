//
// Created by darren on 27/03/2021.
//

#ifndef SPECTRUM_POKEDEFINITION_H
#define SPECTRUM_POKEDEFINITION_H

#include <string>
#include <vector>
#include "../z80/types.h"
#include "basespectrum.h"

namespace Spectrum
{
    class PokeDefinition
    {
        public:
            struct Poke
            {
                ::Z80::UnsignedWord address;
                // any byte whose value is an empty optional should be provided by user
                std::vector<std::optional<::Z80::UnsignedByte>> bytes;
            };

            using Pokes = std::vector<Poke>;

            [[nodiscard]]
            const std::string & name() const
            {
                return m_name;
            }

            [[nodiscard]]
            const std::string & description() const
            {
                return m_description;
            }

            [[nodiscard]]
            const std::string & technicalDetails() const
            {
                return m_techDetails;
            }

            [[nodiscard]]
            const Pokes & pokes() const
            {
                return m_pokes;
            }

            /**
             * The poke definitions that will undo the poke.
             *
             * This will be empty until the poke has been applied.
             *
             * @return
             */
            [[nodiscard]]
            const Pokes & undoPokes() const
            {
                return m_undo;
            }

            void resetPokes() noexcept
            {
                m_undo.clear();
            }

            void setName(std::string name) noexcept
            {
                m_name = std::move(name);
            }

            void setDescription(std::string description) noexcept
            {
                m_description = std::move(description);
            }

            void setTechnicalDetails(std::string details) noexcept
            {
                m_techDetails = std::move(details);
            }

            void addPoke(const Poke & poke)
            {
                m_pokes.push_back(poke);
            }

            void addPoke(Poke && poke)
            {
                m_pokes.push_back(std::move(poke));
            }

            template<std::same_as<::Z80::UnsignedByte> T, std::same_as<::Z80::UnsignedByte>... Ts>
            void addPoke(::Z80::UnsignedWord address, T && byte, Ts &&... bytes)
            {
                m_pokes.push_back({.address = address, .bytes = {byte, bytes...}});
            }

            void addPoke(const ::Z80::UnsignedWord address, std::vector<std::optional<::Z80::UnsignedByte>> bytes)
            {
                m_pokes.push_back({.address = address, .bytes = std::move(bytes)});
            }

            /** Apply the pokes to the given memory. */
            void apply(BaseSpectrum::MemoryType & memory) const;

            void apply(const BaseSpectrum & spectrum) const
            {
                sp_assert(spectrum.memory(), "detected Spectrum with null memory in Spectrum::PokeDefinition::apply()");
                return apply(*(spectrum.memory()));
            }

            /** Restores the poked bytes to their original values. */
            void undo(BaseSpectrum::MemoryType & memory) const;

            void undo(const BaseSpectrum & spectrum) const
            {
                sp_assert(spectrum.memory(), "detected Spectrum with null memory in Spectrum::PokeDefinition::undo()");
                return undo(*(spectrum.memory()));
            }

        private:
            std::string m_name;
            std::string m_description;
            std::string m_techDetails;
            Pokes m_pokes;
            mutable Pokes m_undo;
    };
}

#endif // SPECTRUM_POKEDEFINITION_H
