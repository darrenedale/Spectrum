//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_DEBUGGER_BREAKPOINT_H
#define SPECTRUM_DEBUGGER_BREAKPOINT_H

#include <string>
#include <vector>

namespace Spectrum
{
    class BaseSpectrum;
}

namespace Spectrum::Debugger
{
    /**
     * Abstract base class for Spectrum emulator breakpoints.
     */
    class Breakpoint
    {
    public:
        /**
         * Interface for breakpoint observers.
         */
        class Observer
        {
        public:
            /**
             * Notify the observer that the observed condition has been met.
             */
            virtual void notify(Breakpoint *) = 0;
        };

        /**
         * Default constructor.
         */
        Breakpoint() = default;

        /**
         * Destructor.
         */
        virtual ~Breakpoint() = default;

        /**
         * Fetch the display name for the type of breakpoint.
         *
         * @return The breakpoint type name.
         */
        [[nodiscard]] virtual std::string typeName() const = 0;

        /**
         * Fetch a human-readable description of the breakpoint condition.
         *
         * @return The description.
         */
        [[nodiscard]] virtual std::string conditionDescription() const = 0;

        /**
         * Check whether a breakpoint is equivalent to another.
         *
         * @return true if the two breakpoints are equivalent, false if not.
         */
        virtual bool operator==(const Breakpoint &) const = 0;

        /**
         * Check whether the state of the provided Spectrum meets the breakpoint condition.
         *
         * @return true if the breakpoint's condition is met, false otherwise.
         */
        virtual bool check(const BaseSpectrum &) = 0;

        /**
         * Add an observer for the breakpoint.
         *
         * The base class manages a list of observers. Observers are borrowed not owned. The caller is responsible for removing the observer before it is
         * destroyed. Observers must not be nullptr.
         */
        void addObserver(Observer *);

        /**
         * Remove an observer from the breakpoint.
         *
         * The provided observer will no longer receive notifications from the breakpoint. The observer is not destroyed. It is safe to provide a pointer to an
         * observer that is not currently observing the breakpoint - in this case the call is a no-op. It is also safe to provide nullptr.
         */
        void removeObserver(Observer *);

        /**
         * Check whether the provided observer is observing the breakpoint.
         *
         * @return true if it is, false otherwise.
         */
        bool hasObserver(Observer *) const;

        /**
         * All observers will be removed.
         */
        void clearObservers();

    protected:
        /**
         * Helper to notify all observers when a breakpoint's condition is met.
         *
         * Call this from inside your subclass's check() method if the breakpoint condition is met.
         */
        void notifyObservers();

    private:
        /**
         * Alias for the storage class for the list of observers.
         */
        using Observers = std::vector<Observer *>;

        /**
         * The observers of the breakpoint.
         */
        Observers m_observers;
    };
}

#endif //SPECTRUM_DEBUGGER_BREAKPOINT_H
