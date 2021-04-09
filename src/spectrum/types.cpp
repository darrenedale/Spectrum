//
// Created by darren on 07/04/2021.
//

#include "types.h"

std::string std::to_string(Spectrum::Model model)
{
    switch (model) {
        case Spectrum::Model::Spectrum16k:
            return "Spectrum 16K";

        case Spectrum::Model::Spectrum48k:
            return "Spectrum 48K";

        case Spectrum::Model::Spectrum128k:
            return "Spectrum 128K";

        case Spectrum::Model::SpectrumPlus2a:
            return "Spectrum +2a";
    }
}
