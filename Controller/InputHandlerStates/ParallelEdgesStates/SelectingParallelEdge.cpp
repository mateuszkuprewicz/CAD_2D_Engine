#include "./SelectingParallelEdge.h"
#include "../../../Models/scene.h"
#include "../../../Models/edge.h"
#include "../../../Models/Constraints/parallelismconstraint.h"
#include <memory>

void SelectingParallelEdge::applyParallelAction(Edge* targetEdge) {
    auto constraint = std::make_shared<ParallelismConstraint>(firstParallelEdge, targetEdge);
    scene->AddConstraint(std::move(constraint));
    scene->TrySolveConstraints(firstParallelEdge->vertices.first.get(),
                               firstParallelEdge->vertices.second.get());
}