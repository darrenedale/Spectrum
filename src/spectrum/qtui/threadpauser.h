//
// Created by darren on 07/03/2021.
//

#ifndef SPECTRUM_THREADPAUSER_H
#define SPECTRUM_THREADPAUSER_H

#include "thread.h"

namespace Spectrum::QtUi
{
    /**
     * Utility class to pause a thread for the scope in which the pauser is created.
     *
     * When the pauser goes out of scope, if the thread was running when the pauser was created it will be resumed; otherwise it will remain paused. You can
     * resume the thread early by calling release().
     */
    class ThreadPauser
    {
    public:
        /**
         * Initialise a new pauser with the given thread.
         *
         * The thread will be paused. On destruction of the pauser, or when release() is called, the thread will be resumed if it was running when the pauser
         * was created.
         *
         * @param thread
         */
        explicit ThreadPauser(Thread & thread)
                : m_thread(thread),
                  m_wasPaused(thread.isPaused()),
                  m_released(false)
        {
            thread.pause();
        }

        /**
         * Destructor.
         *
         * Unless already released, the thread will be resumed if it was running when the pauser was created.
         */
        virtual ~ThreadPauser()
        {
            release();
        }

        /**
         * Release the pauser, resuming the thread if it was running when the pauser was created.
         */
        void release()
        {
            if (!m_released && !m_wasPaused) {
                m_thread.resume();
            }

            m_released = true;
        }

    private:
        /**
         * The thread to pause.
         */
        Thread & m_thread;

        /**
         * Whether the thread was in the paused state when the pauser was created.
         */
        bool m_wasPaused;

        /**
         * Whether the thread has already been released from the pauser.
         */
        bool m_released;
    };
}

#endif //SPECTRUM_THREADPAUSER_H
