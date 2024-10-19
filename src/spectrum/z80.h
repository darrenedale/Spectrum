//
// Created by darren on 11/03/2021.
//

#ifndef SPECTRUM_Z80_H
#define SPECTRUM_Z80_H

#include <algorithm>
#include <vector>

#include "../z80/types.h"
#include "../z80/z80.h"

namespace Spectrum
{
    class Z80
    : public ::Z80::Z80
    {
    public:
        using UnsignedByte = ::Z80::UnsignedByte;
        using InstructionCost = ::Z80::InstructionCost;

        class Observer
        {
        public:
            virtual void notify(Z80 * cpu) = 0;
        };

        explicit Z80(MemoryType * memory);
        ~Z80() override = default;

        InstructionCost execute(const UnsignedByte *instruction, bool doPc) override;

        inline void addInstructionObserver(Observer * observer)
        {
            addObserver(m_instructionObservers, observer);
        }

        inline void removeInstructionObserver(Observer * observer)
        {
            removeObserver(m_instructionObservers, observer);
        }

        inline void addNmiObserver(Observer * observer)
        {
            addObserver(m_nmiObservers, observer);
        }

        inline void removeNmiObserver(Observer * observer)
        {
            removeObserver(m_nmiObservers, observer);
        }

        inline void addInterruptObserver(Observer * observer)
        {
            addObserver(m_interruptObservers, observer);
        }

        inline void removeInterruptObserver(Observer * observer)
        {
            removeObserver(m_interruptObservers, observer);
        }

    protected:
        int handleInterrupt() override;
        void handleNmi() override;

    private:
        using Observers = std::vector<Observer *>;

        static void addObserver(Observers & observers, Observer * observer)
        {
            if (observers.cend() != std::find(observers.cbegin(), observers.cend(), observer)) {
                // already observing
                return;
            }

            observers.push_back(observer);
        }

        static void removeObserver(Observers & observers, Observer * observer)
        {
            auto observerIterator = std::find(observers.cbegin(), observers.cend(), observer);

            if (observers.cend() == observerIterator) {
                // not one of our observers
                return;
            }

            observers.erase(observerIterator);
        }

        void notifyObservers(const Observers & observers)
        {
            for (auto * observer : observers) {
                observer->notify(this);
            }
        }

        std::vector<Observer *> m_instructionObservers;
        std::vector<Observer *> m_nmiObservers;
        std::vector<Observer *> m_interruptObservers;
    };
}

#endif //SPECTRUM_Z80_H
