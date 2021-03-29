//
// Created by darren on 07/03/2021.
//

#ifndef SPECTRUM_THREADPAUSER_H
#define SPECTRUM_THREADPAUSER_H

#include "thread.h"

namespace Spectrum::QtUi
{
    class ThreadPauser
    {
    public:
        explicit ThreadPauser(Thread & thread)
        : m_thread(thread),
          m_wasPaused(thread.isPaused())
        {
            thread.pause();
        }

        virtual ~ThreadPauser()
        {
            if (!m_wasPaused) {
                m_thread.resume();
            }
        }

    private:
        Thread & m_thread;
        bool m_wasPaused;
    };
}

#endif //SPECTRUM_THREADPAUSER_H
