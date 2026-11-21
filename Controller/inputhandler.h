#pragma once
#include <memory>
#include <QMouseEvent>
#include "InputHandlerStates/DefaultState/HitTest.h"

class AbstractInputHandlerState;
class Scene;
class Edge;
class QWidget;

class InputHandler
{
public:
    explicit InputHandler(Scene* scene);
    ~InputHandler();

    void ChangeState(std::unique_ptr<AbstractInputHandlerState> newState);

    bool handleDoubleClick(QMouseEvent* event);
    bool handleRightClick(QWidget* parent, QMouseEvent* event);
    bool handleLeftClick(QMouseEvent* event);
    bool handleMouseMove(QMouseEvent* event);
    bool handleMouseRelease(QMouseEvent* event);

    const Edge* getHoveredEdge() const;
    const HitTest& getHitTest() const { return hitTest; }

private:
    std::unique_ptr<AbstractInputHandlerState> state;
    HitTest hitTest;
    Scene* scene = nullptr;
};