#pragma once

class InputHandler;
class Scene;
class Edge;
class QMouseEvent;
class QWidget;

class AbstractInputHandlerState
{
public:
    AbstractInputHandlerState(InputHandler* context, Scene* scene);
    virtual ~AbstractInputHandlerState() = default;

    virtual bool handleDoubleClick(QMouseEvent* event) {return false;}
    virtual bool handleRightClick(QWidget* parent, QMouseEvent* event) {return false;}
    virtual bool handleLeftClick(QMouseEvent* event) {return false;}
    virtual bool handleMouseMove(QMouseEvent* event) {return false;}
    virtual bool handleMouseRelease(QMouseEvent* event) {return false;}

    virtual const Edge* getHoveredEdge() const { return nullptr; }

protected:
    InputHandler* context = nullptr;
    Scene* scene = nullptr;
};