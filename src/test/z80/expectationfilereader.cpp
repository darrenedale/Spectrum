//
// Created by darren on 26/02/2021.
//

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

#include "expectationfilereader.h"

using namespace Test::Z80;

ExpectationFileReader::ExpectationFileReader(std::string fileName)
: FileReader(std::move(fileName))
{
}

std::optional<Expectation> ExpectationFileReader::nextExpectation()
{
    if (!isOpen()) {
        std::cerr << "Failed reading expectation - reader is not open\n";
        return {};
    }

    skipEmptyLines();
    auto name = readName();

    if (!name) {
        // failed to read name
        std::cerr << "Failed reading name for expectation\n";
        return {};
    }

    auto events = readEvents();

    if (!events) {
        std::cerr << "Failed reading events\n";
        return {};
    }

    State state;

    if (!readRegisterPairs(state)) {
        std::cerr << "Failed reading register pairs for expectation " << *name << "\n";
        return {};
    }

    std::size_t tStates;

    if (!readRegistersFlagsTStates(state, tStates)) {
        std::cerr << "Failed reading registers, flags and t-states for expectation " << *name << "\n";
        return {};
    }

    while ('\n' != m_inStream.peek()) {
        auto block = readMemoryBlock();

        if (!block) {
            std::cerr << "Failed reading memory block\n";
            return {};
        }

        state.memory.push_back(std::move(*block));
    }

    skipEmptyLines();
    return Expectation(std::move(*name), tStates, std::move(*events), std::move(state));
}

std::optional<std::vector<Event>> ExpectationFileReader::readEvents()
{
    std::vector<Event> events;

    while (' ' == m_inStream.peek()) {
        auto event = readEvent();

        if (!event) {
            std::cerr << "Error reading event\n";
            return {};
        }

        events.push_back(*event);
    }

    return std::move(events);
}

std::optional<Event> ExpectationFileReader::readEvent()
{
    auto line = readLine();

    if (!line) {
        std::cerr << "Error reading event line\n";
        return {};
    }

    std::istringstream in(*line);
    Event event = {};
    in >> event.time;

    if (' ' != in.get()) {
        std::cerr << "Invalid separator between time and event type\n";
        return {};
    }

    char buffer[4];
    in.read(buffer, 2);

    if (2 != in.gcount()) {
        std::cerr << "Error reading event type\n";
        return {};
    }

    if (' ' != in.get()) {
        std::cerr << "Invalid separator between event type and address/port\n";
        return {};
    }

    if (0 == std::strncmp("MC", buffer, 2)) {
        event.type = EventType::MemoryContend;
    } else if (0 == std::strncmp("MR", buffer, 2)) {
        event.type = EventType::MemoryRead;
    } else if (0 == std::strncmp("MW", buffer, 2)) {
        event.type = EventType::MemoryWrite;
    } else if (0 == std::strncmp("PC", buffer, 2)) {
        event.type = EventType::PortContend;
    } else if (0 == std::strncmp("PR", buffer, 2)) {
        event.type = EventType::PortRead;
    } else if (0 == std::strncmp("PW", buffer, 2)) {
        event.type = EventType::PortWrite;
    } else {
        std::cerr << "Unrecognised event type '" << buffer[0] << buffer[1] << "'\n";
        return {};
    }

    in.read(buffer, 4);

    if (4 != in.gcount()) {
        std::cerr << "Error reading address/port\n";
        return {};
    }

    auto separator = in.get();

    if (' ' != separator && (-1 != separator || !in.eof())) {
        std::cerr << "Invalid separator between address/port and data\n";
        return {};
    }

    auto addr = parseWord(buffer);

    if (!addr) {
        std::cerr << "Failed parsing address '" << buffer[0] << buffer[1] << buffer[2] << buffer[3] << "'\n";
        return {};
    }

    event.address = *addr;

    if (' ' == separator) {
        in.read(buffer, 2);

        if (2 != in.gcount()) {
            std::cerr << "Error reading event data\n";
            return {};
        }

        if (-1 != in.get() || !in.eof()) {
            std::cerr << "Extraneous content after event data\n";
            return {};
        }

        auto data = parseByte(buffer);

        if (!data) {
            std::cerr << "Failed parsing data '" << buffer[0] << buffer[1] << "'\n";
            return {};
        }

        event.data = *data;
    }

    return event;
}
