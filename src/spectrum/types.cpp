//
// Created by darren on 07/04/2021.
//

#include <cassert>
#include "types.h"

using Spectrum::Colour;
using Spectrum::ScreenBuffer128k;
using Spectrum::Model;

std::string std::to_string(Model model)
{
    switch (model) {
        case Model::Spectrum16k:
            return "Spectrum 16K"s;

        case Model::Spectrum48k:
            return "Spectrum 48K"s;

        case Model::Spectrum128k:
            return "Spectrum 128K"s;

        case Model::SpectrumPlus2:
            return "Spectrum +2"s;

        case Model::SpectrumPlus2a:
            return "Spectrum +2a"s;
    }

    // unreachable code
    assert(false);
}

std::string std::to_string(ScreenBuffer128k bufferType)
{
    switch (bufferType) {
        case ScreenBuffer128k::Shadow:
            return "Shadow buffer"s;

        case ScreenBuffer128k::Normal:
            return "Normal buffer"s;
    }

    // unreachable code - someone has added a buffer type and hasn't updated the function or has type punned an invalid
    // value to a buffer type
    assert(false);
}

std::string std::to_string(Colour colour)
{
    switch (colour) {
        case Colour::Black:
            return "Black"s;

        case Colour::Blue:
            return "Blue"s;

        case Colour::Red:
            return "Red"s;

        case Colour::Magenta:
            return "Magenta"s;

        case Colour::Green:
            return "Green"s;

        case Colour::Cyan:
            return "Cyan"s;

        case Colour::Yellow:
            return "Yellow"s;

        case Colour::White:
            return "Black"s;
    }

    // unreachable code - someone has added a colour and hasn't updated the function or has type punned an invalid
    // value to a colour
    assert(false);
}

std::ostream & Spectrum::operator<<(std::ostream & out, Spectrum::Colour colour)
{
    out << std::to_string(colour);
    return out;
}

std::ostream & Spectrum::operator<<(std::ostream & out, Spectrum::Model model)
{
    out << std::to_string(model);
    return out;
}

std::ostream & Spectrum::operator<<(std::ostream & out, Spectrum::ScreenBuffer128k bufferType)
{
    out << std::to_string(bufferType);
    return out;
}
