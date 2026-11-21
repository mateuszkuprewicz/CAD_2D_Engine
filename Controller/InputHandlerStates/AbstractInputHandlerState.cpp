#include "./abstractinputhandlerstate.h"

AbstractInputHandlerState::AbstractInputHandlerState(InputHandler* context, Scene* scene)
    : context(context), scene(scene) {}