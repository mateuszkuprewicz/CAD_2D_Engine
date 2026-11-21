#include "./DeletingParallelEdge.h"
#include "../../../Models/scene.h"

void DeletingParallelEdge::applyParallelAction(Edge* targetEdge) {
    if (auto* c = scene->constraintManager.GetParallelConstraint(firstParallelEdge, targetEdge)) {
        scene->DelConstraint(c);
    }
}