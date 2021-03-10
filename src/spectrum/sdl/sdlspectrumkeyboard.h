//
// Created by darren on 10/03/2021.
//

#ifndef SPECTRUM_SDLSPECTRUMKEYBOARD_H
#define SPECTRUM_SDLSPECTRUMKEYBOARD_H

#include "../keyboard.h"

namespace Spectrum::Sdl
{
    class SdlSpectrumKeyboard
    : public SpectrumKeyboard
    {
    public:
        SdlSpectrumKeyboard();

        [[nodiscard]] bool keyState(Key key) const override;

    private:
        int mapKeyToScancode(SpectrumKeyboard::Key key) const;
    };
}

#endif //SPECTRUM_SDLSPECTRUMKEYBOARD_H
