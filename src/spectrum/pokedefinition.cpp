//
// Created by darren on 27/03/2021.
//

#include "pokedefinition.h"

void Spectrum::PokeDefinition::apply(::Spectrum::BaseSpectrum::MemoryType & memory) const
{
    m_undo.clear();

    for (const auto & poke : pokes()) {
        Poke undoPoke = {.address = poke.address, .bytes = {}};
        auto address = poke.address;

        for (const auto & byte : poke.bytes) {
            undoPoke.bytes.emplace_back(memory.readByte(address));
            // TODO need a mechanism by which user-provided values can be inserted where byte is an unfilled optional
            memory.writeByte(address, *byte);
            ++address;
        }

        m_undo.push_back(std::move(undoPoke));
    }
}

void Spectrum::PokeDefinition::undo(::Spectrum::BaseSpectrum::MemoryType & memory) const
{
    for (const auto & poke : undoPokes()) {
        auto address = poke.address;

        for (const auto & byte : poke.bytes) {
            memory.writeByte(address, *byte);
            ++address;
        }
    }
}
