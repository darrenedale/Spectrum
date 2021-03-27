//
// Created by darren on 27/03/2021.
//

#include "pokedefinition.h"

Spectrum::PokeDefinition Spectrum::PokeDefinition::apply(::Spectrum::BaseSpectrum::MemoryType & memory) const
{
    PokeDefinition undo;
    undo.m_name = "Undo " + name();
    undo.m_description = "Undo the pokes applied by " + name();
    undo.m_techDetails = name() + " applied the following changes:\n" + technicalDetails();

    for (const auto & poke : pokes()) {
        Poke undoPoke = {.address = poke.address, .bytes = {}};
        auto address = poke.address;

        for (const auto & byte : poke.bytes) {
            undoPoke.bytes.emplace_back(memory.readByte(address));
            // TODO need a mechanism by which user-provided values can be inserted where byte is an unfilled optional
            memory.writeByte(address, *byte);
            ++address;
        }
    }

    return undo;
}
