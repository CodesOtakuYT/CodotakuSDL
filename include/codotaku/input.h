#ifndef CODOTAKU_INPUT_H
#define CODOTAKU_INPUT_H

#include <SDL3/SDL_events.h>
#include <glm/vec2.hpp>

namespace codotaku
{

struct Input
{
    glm::vec2 mousePos{0.0f};
    glm::vec2 mouseDelta{0.0f};
    float mouseWheel = 0.0f;
    bool mouseLeft = false;

    void handleEvent(const SDL_Event &event) noexcept
    {
        switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
            mouseDelta.x += event.motion.xrel;
            mouseDelta.y += event.motion.yrel;
            mousePos.x = event.motion.x;
            mousePos.y = event.motion.y;
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
        case SDL_EVENT_MOUSE_WHEEL:
            mouseWheel += event.wheel.y;
            break;
        }
    }

    void endFrame() noexcept
    {
        mouseDelta = {};
        mouseWheel = 0.0f;
    }
};

} // namespace codotaku

#endif
