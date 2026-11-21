#pragma once
#include "../AbstractInputHandlerState.h"
#include <QPoint>

class Vertex;

class DraggingVertex : public AbstractInputHandlerState
{
public:
    DraggingVertex(InputHandler* context, Scene* scene, Vertex* draggedVertex, QPoint startPos);
    bool handleMouseMove(QMouseEvent* event) override;
    bool handleMouseRelease(QMouseEvent* event) override;

private:
    Vertex* draggedVertex = nullptr;
    QPoint lastMousePos;
};