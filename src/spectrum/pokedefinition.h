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

        [[nodiscard]] const std::string & name() const
        {
            return m_name;
        }

        [[nodiscard]] const std::string & description() const
        {
            return m_description;
        }

        [[nodiscard]] const std::string & technicalDetails() const
        {
            return m_techDetails;
        }

        [[nodiscard]] const Pokes & pokes() const
        {
            return m_pokes;
        }

        void setName(std::string name)
        {
            m_name = std::move(name);
        }

        void setDescription(std::string description)
        {
            m_name = std::move(description);
        }

        void setTechnicalDetails(std::string details)
        {
            m_name = std::move(details);
        }

        void addPoke(const Poke & poke)
        {
            m_pokes.push_back(poke);
        }

        void addPoke(Poke && poke)
        {
            m_pokes.push_back(std::move(poke));
        }

        void addPoke(::Z80::UnsignedWord address, ::Z80::UnsignedByte bytes...)
        {
            m_pokes.push_back({.address = address, .bytes = {bytes}});
        }

        void addPoke(::Z80::UnsignedWord address, std::vector<std::optional<::Z80::UnsignedByte>> bytes)
        {
            m_pokes.push_back({.address = address, .bytes = std::move(bytes)});
        }

        /**
         * Returns a PokeDefinition to apply to undo the poke.
         *
         * @param memory
         * @return
         */
        PokeDefinition apply(BaseSpectrum::MemoryType & memory) const;

        PokeDefinition apply(BaseSpectrum & spectrum) const
        {
            assert(spectrum.memory());
            return apply(*(spectrum.memory()));
        }

    private:
        std::string m_name;
        std::string m_description;
        std::string m_techDetails;
        Pokes m_pokes;
    };
}

#endif //SPECTRUM_POKEDEFINITION_H
