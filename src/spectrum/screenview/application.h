//
// Created by darren on 25/02/2021.
//

#ifndef SPECTRUM_APPLICATION_H
#define SPECTRUM_APPLICATION_H

#include <QApplication>

namespace ScreenView
{
    class Application : public QApplication
    {
    public:
        Application(int & argc, char ** argv);
    };
}

#endif //SPECTRUM_APPLICATION_H
