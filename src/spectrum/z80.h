//
// Created by darren on 11/03/2021.
//

#ifndef SPECTRUM_Z80_H
#define SPECTRUM_Z80_H

#include <algorithm>
#include <utility>
#include <vector>

#include "../z80/types.h"
#include "../z80/z80.h"

namespace Spectrum
{
    namespace CoreZ80 = Z80;
    using CoreZ80Cpu = CoreZ80::Z80;

    // TODO consider using unique pointers for observer storage
    class Z80
    : public CoreZ80Cpu
    {
    public:
        using UnsignedByte = CoreZ80::UnsignedByte;
        using InstructionCost = CoreZ80::InstructionCost;

        class Observer
        {
            public:
                virtual ~Observer() = default;
                virtual void notify(Z80 * cpu) = 0;
        };

        explicit Z80(MemoryType * memory);
        ~Z80() override = default;

        InstructionCost execute(const UnsignedByte *instruction, bool doPc) override;

        void addInstructionObserver(Observer * observer)
        {
            addObserver(m_instructionObservers, observer);
        }

        void removeInstructionObserver(Observer * observer)
        {
            removeObserver(m_instructionObservers, observer);
        }

        void addNmiObserver(Observer * observer)
        {
            addObserver(m_nmiObservers, observer);
        }

        void removeNmiObserver(Observer * observer)
        {
            removeObserver(m_nmiObservers, observer);
        }

        void addInterruptObserver(Observer * observer)
        {
            addObserver(m_interruptObservers, observer);
        }

        void removeInterruptObserver(Observer * observer)
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
            if (observers.cend() != std::ranges::find(std::as_const(observers), observer)) {
                // already observing
                return;
            }

            observers.push_back(observer);
        }

        static void removeObserver(Observers & observers, Observer * observer)
        {
            const auto observerIterator = std::ranges::find(std::as_const(observers), observer);

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
