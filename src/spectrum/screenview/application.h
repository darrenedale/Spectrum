//
// Created by darren on 25/02/2021.
//

#ifndef SPECTRUM_SCREENVIEW_APPLICATION_H
#define SPECTRUM_SCREENVIEW_APPLICATION_H

#include <QApplication>

namespace Spectrum::ScreenView
{
    /**
     * Application class for the ScreenView application.
     */
    class Application
    : public QApplication
    {
    public:
        /**
         * Initialise a new Application object.
         *
         * @param argc Reference to the number of args received in main().
         * @param argv The args received in main().
         */
        Application(int & argc, char ** argv);
    };
}

#endif //SPECTRUM_SCREENVIEW_APPLICATION_H
