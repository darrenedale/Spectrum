//
// Created by darren on 27/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_EVENT_H
#define SPECTRUM_TEST_Z80_EVENT_H

#include <vector>

#include "../../z80/z80.h"

namespace Test::Z80
{
    enum class EventType
    {
        MemoryContend,
        MemoryRead,
        MemoryWrite,
        PortContend,
        PortRead,
        PortWrite,
    };

    struct Event
    {
        std::size_t time;
        EventType type;
        ::Z80::UnsignedWord address;
        std::optional<::Z80::UnsignedByte> data;
    };

    using Events = std::vector<Event>;
}

#endif //SPECTRUM_TEST_Z80_EVENT_H
