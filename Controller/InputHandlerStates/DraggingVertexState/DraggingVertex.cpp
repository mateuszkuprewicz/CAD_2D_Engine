#include "./draggingvertex.h"
#include "../../inputhandler.h"
#include "../DefaultState/Default.h"
#include "../../../Models/scene.h"
#include "../../../Models/polygon.h"
#include "../../../Models/edge.h"
#include "../../../Models/vertex.h"
#include "../../../Models/edgetypes.h"
#include <QMouseEvent>
#include <memory>

DraggingVertex::DraggingVertex(InputHandler* context, Scene* scene, Vertex* draggedVertex, QPoint startPos)
    : AbstractInputHandlerState(context, scene), draggedVertex(draggedVertex), lastMousePos(startPos) {}

bool DraggingVertex::handleMouseMove(QMouseEvent* event) {
    if (!scene || !draggedVertex) return false;

    const QPoint currentPos = event->pos();
    const int dx = currentPos.x() - lastMousePos.x();
    const int dy = currentPos.y() - lastMousePos.y();

    draggedVertex->point.rx() += dx;
    draggedVertex->point.ry() += dy;

    for (const auto& poly : scene->Polygons) {
        for (const auto& edge : poly->edges) {
            if (edge->isBezier()) {
                if (edge->vertices.first.get() == draggedVertex) {
                    edge->bezier->controlP1->point.rx() += dx;
                    edge->bezier->controlP1->point.ry() += dy;
                }
                if (edge->vertices.second.get() == draggedVertex) {
                    edge->bezier->controlP2->point.rx() += dx;
                    edge->bezier->controlP2->point.ry() += dy;
                }
            }
        }
    }

    scene->SolveConstraints(draggedVertex);
    lastMousePos = currentPos;
    return true;
}

bool DraggingVertex::handleMouseRelease(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        context->ChangeState(std::make_unique<Default>(context, scene));
        return true;
    }
    return false;
}