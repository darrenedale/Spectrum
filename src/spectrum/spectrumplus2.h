//
// Created by darren on 09/04/2021.
//

#ifndef SPECTRUM_SPECTRUMPLUS2_H
#define SPECTRUM_SPECTRUMPLUS2_H

#include "spectrum128k.h"

namespace Spectrum
{
    /**
     * Spectrum +2 is effectively a re-boxed 128k with a built-in Interface2 joystick.
     *
     * We don't currently forcibly add the interface 2 joystick device, so this is at the moment just a Spectrum 128K
     * that reports the +2 model.
     */
    class SpectrumPlus2
    : public Spectrum128k
    {
    public:
        using Spectrum128k::Spectrum128k;
        ~SpectrumPlus2() override;

        [[nodiscard]] inline Model model() const override
        {
            return Model::SpectrumPlus2;
        }
    };
}

#endif //SPECTRUM_SPECTRUMPLUS2_H