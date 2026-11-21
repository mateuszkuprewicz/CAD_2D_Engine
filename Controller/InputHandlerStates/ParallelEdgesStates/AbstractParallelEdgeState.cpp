#include "./abstractparalleledgestate.h"
#include "../../inputhandler.h"
#include "../DefaultState/default.h"
#include <QMouseEvent>
#include <memory>

AbstractParallelEdgeState::AbstractParallelEdgeState(InputHandler* context,
                                                     Scene* scene,
                                                     Edge* firstParallelEdge,
                                                     Polygon* firstParallelPoly)
    : AbstractInputHandlerState(context, scene),
      firstParallelEdge(firstParallelEdge),
      firstParallelPoly(firstParallelPoly) {}

bool AbstractParallelEdgeState::handleLeftClick(QMouseEvent* event) {
    if (!scene || event->button() != Qt::LeftButton) return false;

    if (hoveredEdge && hoveredEdge != firstParallelEdge) {
        applyParallelAction(hoveredEdge);
    }

    context->ChangeState(std::make_unique<Default>(context, scene));
    return true;
}

bool AbstractParallelEdgeState::handleMouseMove(QMouseEvent* event) {
    if (!scene) return false;

    Edge* foundEdge = context->getHitTest().EdgeHitTest(scene, event->pos());
    if (foundEdge != hoveredEdge) {
        hoveredEdge = foundEdge;
        return true;
    }

    return false;
}