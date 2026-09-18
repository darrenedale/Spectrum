//
// Created by darren on 12/03/2021.
//

#include <algorithm>
#include <cassert>
#include <ranges>
#include <utility>

#include "breakpoint.h"

using namespace Spectrum::Debugger;

void Breakpoint::notifyObservers()
{
    for (auto * observer : m_observers) {
        assert(observer);
        observer->notify(this);
    }
}

void Breakpoint::addObserver(Observer * observer)
{
    assert(observer);
    m_observers.push_back(observer);
}

void Breakpoint::clearObservers()
{
    m_observers.clear();
}

void Breakpoint::removeObserver(const Observer * observer)
{
    if (!observer) {
        return;
    }

    while (true) {
        const auto pos = std::ranges::find(std::as_const(m_observers), observer);

        if (pos == m_observers.cend()) {
            break;
        }

        m_observers.erase(pos);
    }
}

bool Breakpoint::hasObserver(const Observer * observer) const
{
    return m_observers.cend() != std::ranges::find(m_observers, observer);
}
