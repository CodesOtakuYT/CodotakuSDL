#ifndef CODOTAKU_INPUT_H
#define CODOTAKU_INPUT_H

#include <SDL3/SDL_events.h>

namespace codotaku
{

struct Input
{
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    float mouseDX = 0.0f;
    float mouseDY = 0.0f;
    bool mouseLeft = false;

    void handleEvent(const SDL_Event &event) noexcept
    {
        switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
            mouseDX += event.motion.xrel;
            mouseDY += event.motion.yrel;
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseLeft = true;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseLeft = false;
            }
            break;
        }
    }

    void endFrame() noexcept
    {
        mouseDX = 0.0f;
        mouseDY = 0.0f;
    }
};

} // namespace codotaku

#endif
