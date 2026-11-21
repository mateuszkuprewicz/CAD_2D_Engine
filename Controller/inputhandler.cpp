#include "inputhandler.h"
#include "InputHandlerStates/AbstractInputHandlerState.h"
#include "InputHandlerStates/DefaultState/Default.h"

InputHandler::InputHandler(Scene* scene)
    : state(std::make_unique<Default>(this, scene)), scene(scene) {}

InputHandler::~InputHandler() = default;

void InputHandler::ChangeState(std::unique_ptr<AbstractInputHandlerState> newState) {
    state = std::move(newState);
}

bool InputHandler::handleDoubleClick(QMouseEvent* event) {
    return state ? state->handleDoubleClick(event) : false;
}

bool InputHandler::handleRightClick(QWidget* parent, QMouseEvent* event) {
    return state ? state->handleRightClick(parent, event) : false;
}

bool InputHandler::handleLeftClick(QMouseEvent* event) {
    return state ? state->handleLeftClick(event) : false;
}

bool InputHandler::handleMouseMove(QMouseEvent* event) {
    return state ? state->handleMouseMove(event) : false;
}

bool InputHandler::handleMouseRelease(QMouseEvent* event) {
    return state ? state->handleMouseRelease(event) : false;
}

const Edge* InputHandler::getHoveredEdge() const {
    return state ? state->getHoveredEdge() : nullptr;
}