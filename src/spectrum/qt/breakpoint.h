//
// Created by darren on 12/03/2021.
//

#ifndef SPECTRUM_QT_BREAKPOINT_H
#define SPECTRUM_QT_BREAKPOINT_H

#include <QObject>

#include "thread.h"
#include "../../z80/types.h"
#include "../spectrum.h"

namespace Spectrum::Qt
{
    class Breakpoint
    : public QObject
    {
        Q_OBJECT


    public:
        explicit Breakpoint(Thread & thread, QObject * parent = nullptr)
        : QObject(parent),
          m_thread(thread)
        {}

        [[nodiscard]] Thread & thread()
        {
            return m_thread;
        }

        virtual bool check(const Spectrum &) = 0;

    Q_SIGNALS:
        void triggered();

    protected:
        virtual void trigger();

    private:
        Thread & m_thread;
    };
}

#endif //SPECTRUM_QT_BREAKPOINT_H
