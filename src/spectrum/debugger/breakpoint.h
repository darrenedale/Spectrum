//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_BREAKPOINT_H
#define SPECTRUM_DEBUGGER_BREAKPOINT_H

#include <QObject>

#include "../qtui/thread.h"
#include "../../z80/types.h"
#include "../basespectrum.h"

namespace Spectrum::Debugger
{
    class Breakpoint
    {
    public:
        class Observer
        {
        public:
            virtual void notify(Breakpoint *) = 0;
        };

        Breakpoint() = default;
        virtual ~Breakpoint() = default;

        virtual std::string typeName() const = 0;
        virtual std::string conditionDescription() const = 0;

        virtual bool operator==(const Breakpoint &) const = 0;
        virtual bool check(const BaseSpectrum &) = 0;
        void addObserver(Observer *);

    protected:
        void notifyObservers();

    private:
        using Observers = std::vector<Observer *>;
        Observers m_observers;
    };
}

#endif //SPECTRUM_DEBUGGER_BREAKPOINT_H
