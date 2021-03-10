//
// Created by darren on 10/03/2021.
//

#include <SDL2/SDL.h>
#include "sdlspectrumkeyboard.h"

using namespace Spectrum::Sdl;

bool SdlSpectrumKeyboard::keyState(SpectrumKeyboard::Key key) const
{
    // TODO move this call to the readByte() method so that it's not called for every key state request, only when
    //  the port is read
    SDL_PumpEvents();
    auto * keyStates = SDL_GetKeyboardState(nullptr);

    switch (key) {
        case SpectrumKeyboard::Key::Num1:
            return keyStates[SDL_SCANCODE_1] || keyStates[SDL_SCANCODE_KP_1];

        case SpectrumKeyboard::Key::Num2:
            return keyStates[SDL_SCANCODE_2];

        case SpectrumKeyboard::Key::Num3:
            return keyStates[SDL_SCANCODE_3];

        case SpectrumKeyboard::Key::Num4:
            return keyStates[SDL_SCANCODE_4];

        case SpectrumKeyboard::Key::Num5:
            return keyStates[SDL_SCANCODE_5];

        case SpectrumKeyboard::Key::Num6:
            return keyStates[SDL_SCANCODE_6];

        case SpectrumKeyboard::Key::Num7:
            return keyStates[SDL_SCANCODE_7];

        case SpectrumKeyboard::Key::Num8:
            return keyStates[SDL_SCANCODE_8];

        case SpectrumKeyboard::Key::Num9:
            return keyStates[SDL_SCANCODE_9];

        case SpectrumKeyboard::Key::Num0:
            return keyStates[SDL_SCANCODE_0];

        case SpectrumKeyboard::Key::Q:
            return keyStates[SDL_SCANCODE_Q];

        case SpectrumKeyboard::Key::W:
            return keyStates[SDL_SCANCODE_W];

        case SpectrumKeyboard::Key::E:
            return keyStates[SDL_SCANCODE_E];

        case SpectrumKeyboard::Key::R:
            return keyStates[SDL_SCANCODE_R];

        case SpectrumKeyboard::Key::T:
            return keyStates[SDL_SCANCODE_T];

        case SpectrumKeyboard::Key::Y:
            return keyStates[SDL_SCANCODE_Y];

        case SpectrumKeyboard::Key::U:
            return keyStates[SDL_SCANCODE_U];

        case SpectrumKeyboard::Key::I:
            return keyStates[SDL_SCANCODE_I];

        case SpectrumKeyboard::Key::O:
            return keyStates[SDL_SCANCODE_O];

        case SpectrumKeyboard::Key::P:
            return keyStates[SDL_SCANCODE_P];

        case SpectrumKeyboard::Key::A:
            return keyStates[SDL_SCANCODE_A];

        case SpectrumKeyboard::Key::S:
            return keyStates[SDL_SCANCODE_S];

        case SpectrumKeyboard::Key::D:
            return keyStates[SDL_SCANCODE_D];

        case SpectrumKeyboard::Key::F:
            return keyStates[SDL_SCANCODE_F];

        case SpectrumKeyboard::Key::G:
            return keyStates[SDL_SCANCODE_G];

        case SpectrumKeyboard::Key::H:
            return keyStates[SDL_SCANCODE_H];

        case SpectrumKeyboard::Key::J:
            return keyStates[SDL_SCANCODE_J];

        case SpectrumKeyboard::Key::K:
            return keyStates[SDL_SCANCODE_K];

        case SpectrumKeyboard::Key::L:
            return keyStates[SDL_SCANCODE_L];

        case SpectrumKeyboard::Key::Enter:
            return keyStates[SDL_SCANCODE_KP_ENTER] || keyStates[SDL_SCANCODE_RETURN] || keyStates[SDL_SCANCODE_RETURN2];

        case SpectrumKeyboard::Key::CapsShift:
            return keyStates[SDL_SCANCODE_LSHIFT] || keyStates[SDL_SCANCODE_RSHIFT];

        case SpectrumKeyboard::Key::Z:
            return keyStates[SDL_SCANCODE_Z];

        case SpectrumKeyboard::Key::X:
            return keyStates[SDL_SCANCODE_X];

        case SpectrumKeyboard::Key::C:
            return keyStates[SDL_SCANCODE_C];

        case SpectrumKeyboard::Key::V:
            return keyStates[SDL_SCANCODE_V];

        case SpectrumKeyboard::Key::B:
            return keyStates[SDL_SCANCODE_B];

        case SpectrumKeyboard::Key::N:
            return keyStates[SDL_SCANCODE_N];

        case SpectrumKeyboard::Key::M:
            return keyStates[SDL_SCANCODE_M];

        case SpectrumKeyboard::Key::SymbolShift:
            return keyStates[SDL_SCANCODE_LALT] || keyStates[SDL_SCANCODE_RALT];

        case SpectrumKeyboard::Key::Space:
            return keyStates[SDL_SCANCODE_SPACE] || keyStates[SDL_SCANCODE_KP_SPACE];
    }

    return false;
}

SdlSpectrumKeyboard::SdlSpectrumKeyboard()
{
    static bool initDone = false;

    if (!initDone) {
        if (SDL_Init(0) || SDL_InitSubSystem(SDL_INIT_VIDEO)) {
            std::cerr << "failed to initialise SDL subsystem\n";
        }

        initDone = true;
    }
}
