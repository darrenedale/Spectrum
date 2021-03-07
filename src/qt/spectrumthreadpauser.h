//
// Created by darren on 07/03/2021.
//

#ifndef SPECTRUM_SPECTRUMTHREADPAUSER_H
#define SPECTRUM_SPECTRUMTHREADPAUSER_H

#include "spectrumthread.h"

namespace Spectrum
{

    class SpectrumThreadPauser
    {
    public:
        SpectrumThreadPauser(SpectrumThread & thread)
        : m_thread(thread),
          m_wasPaused(thread.isPaused())
        {
            thread.pause();
        }

        virtual ~SpectrumThreadPauser()
        {
            if (m_wasPaused) {
                m_thread.resume();
            }
        }

    private:
        SpectrumThread & m_thread;
        bool m_wasPaused;
    };
}

#endif //SPECTRUM_SPECTRUMTHREADPAUSER_H
