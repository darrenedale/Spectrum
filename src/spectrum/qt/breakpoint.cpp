//
// Created by darren on 12/03/2021.
//

#include "breakpoint.h"

using namespace Spectrum::Qt;

void Breakpoint::trigger()
{
    m_thread.pause();
    Q_EMIT triggered();
}
