//
// Created by darren on 12/03/2021.
//

#include <algorithm>
#include <cassert>
#include "breakpoint.h"

using namespace Spectrum::Debugger;

void Breakpoint::notifyObservers()
{
    for (auto * observer : m_observers) {
        assert(observer);
        observer->notify(this);
    }
}

void Breakpoint::addObserver(Breakpoint::Observer * observer)
{
    assert(observer);
    m_observers.push_back(observer);
}

void Breakpoint::clearObservers()
{
    m_observers.clear();
}

void Breakpoint::removeObserver(Observer * observer)
{
    if (!observer) {
        return;
    }

    while (true) {
        const auto pos = std::find(m_observers.cbegin(), m_observers.cend(), observer);

        if (pos == m_observers.cend()) {
            break;
        }

        m_observers.erase(pos);
    }
}

bool Breakpoint::hasObserver(Observer * observer) const
{
    return m_observers.cend() != std::find(m_observers.cbegin(), m_observers.cend(), observer);
}
