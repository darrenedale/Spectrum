//
// Created by darren on 12/03/2021.
//

#include <algorithm>
#include <print>
#include <ranges>
#include <utility>

#include "breakpoint.h"
#include "../../util/assert.h"

using namespace Spectrum::Debugger;

void Breakpoint::notifyObservers()
{
    for (auto * observer : m_observers) {
        sp_assert(observer, "found null observer in Breakpoint::notifyObservers");
        observer->notify(this);
    }
}

void Breakpoint::addObserver(Observer * observer)
{
        sp_assert(observer, "found null observer in Breakpoint::addObserver");
    m_observers.push_back(observer);
}

void Breakpoint::clearObservers() noexcept
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

bool Breakpoint::hasObserver(const Observer * observer) const noexcept
{
    return m_observers.cend() != std::ranges::find(m_observers, observer);
}
