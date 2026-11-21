#include "DraggingPolygon.h"
#include "../../inputhandler.h"
#include "../DefaultState/Default.h"
#include "../../../Models/polygon.h"
#include <QMouseEvent>
#include <memory>

DraggingPolygon::DraggingPolygon(InputHandler* context, Scene* scene, Polygon* draggedPolygon, QPoint startPos)
    : AbstractInputHandlerState(context, scene), draggedPolygon(draggedPolygon), lastMousePos(startPos) {}

bool DraggingPolygon::handleMouseMove(QMouseEvent* event) {
    if (!scene || !draggedPolygon) return false;

    const QPoint currentPos = event->pos();
    const int dx = currentPos.x() - lastMousePos.x();
    const int dy = currentPos.y() - lastMousePos.y();

    draggedPolygon->translate(dx, dy);
    lastMousePos = currentPos;
    return true;
}

bool DraggingPolygon::handleMouseRelease(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        context->ChangeState(std::make_unique<Default>(context, scene));
        return true;
    }
    return false;
}