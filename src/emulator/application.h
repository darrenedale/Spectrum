//
// Created by darren on 25/02/2021.
//

#ifndef SPECTRUM_EMULATOR_APPLICATION_H
#define SPECTRUM_EMULATOR_APPLICATION_H

#include <QApplication>

namespace Spectrum
{
    class Application : public QApplication
    {
    public:
        Application(int & argc, char ** argv);
    };
}

#endif //SPECTRUM_EMULATOR_APPLICATION_H
