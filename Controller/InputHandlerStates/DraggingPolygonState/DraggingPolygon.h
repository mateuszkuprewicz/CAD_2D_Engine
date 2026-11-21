#pragma once
#include "../AbstractInputHandlerState.h"
#include <QPoint>

class Polygon;

class DraggingPolygon : public AbstractInputHandlerState
{
public:
    DraggingPolygon(InputHandler* context, Scene* scene, Polygon* draggedPolygon, QPoint startPos);
    bool handleMouseMove(QMouseEvent* event) override;
    bool handleMouseRelease(QMouseEvent* event) override;

private:
    Polygon* draggedPolygon = nullptr;
    QPoint lastMousePos;
};