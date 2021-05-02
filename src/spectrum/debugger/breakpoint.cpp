//
// Created by darren on 12/03/2021.
//

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
    m_observers.push_back(observer);
}
