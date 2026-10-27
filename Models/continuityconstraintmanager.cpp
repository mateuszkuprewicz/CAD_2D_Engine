#include "continuityconstraintmanager.h"
#include "constraintmanager.h"
#include "Constraints/continuityconstraint.h"
#include "edge.h"
#include "vertex.h"
#include "scene.h"
#include "polygon.h"

std::vector<std::shared_ptr<AbstractConstraint>> ContinuityConstraintManager::HandleEdgeChange(ConstraintManager* cm, Vertex* sharedVertex, Edge* unchangedEdge) {
    std::vector<std::shared_ptr<AbstractConstraint>> survivingConstraints;
    if (!cm || !sharedVertex || !unchangedEdge) return survivingConstraints;

    auto it = cm->Constraints.find(sharedVertex);
    if (it == cm->Constraints.end()) return survivingConstraints;

    std::vector<std::shared_ptr<AbstractConstraint>> allContinuity;

    for (const auto& c : it->second) {
        if (auto* contC = dynamic_cast<ContinuityConstraint*>(c.get())) {
            if (contC->getVertex() == sharedVertex) {
                allContinuity.push_back(c);
                bool isLine = !unchangedEdge->isBezier() && !unchangedEdge->isArc();
                bool isBezier = unchangedEdge->isBezier();
                bool isArc = unchangedEdge->isArc();
                bool shouldDelete = false;

                if (isLine) {
                    shouldDelete = true;
                } else if (isBezier) {
                    shouldDelete = false;
                } else if (isArc) {
                    if (dynamic_cast<C1ContinuityConstraint*>(c.get())) {
                        shouldDelete = true;
                    } else if (dynamic_cast<G1ContinuityConstraint*>(c.get())) {
                        shouldDelete = false;
                    }
                }

                if (!shouldDelete) {
                    survivingConstraints.push_back(c);
                }
            }
        }
    }

    for (const auto& c : allContinuity) {
        cm->DelConstraint(c.get());
    }

    return survivingConstraints;
}

ContinuityState ContinuityConstraintManager::SaveState(ConstraintManager* cm, Vertex* v) {
    ContinuityState state;
    if (!cm || !v) return state;

    auto it = cm->Constraints.find(v);
    if (it != cm->Constraints.end()) {
        for (const auto& c : it->second) {
            if (auto* cont = dynamic_cast<ContinuityConstraint*>(c.get())) {
                if (cont->getVertex() == v) {
                    if (dynamic_cast<G1ContinuityConstraint*>(c.get())) state.isG1 = true;
                    if (dynamic_cast<C1ContinuityConstraint*>(c.get())) state.isC1 = true;
                }
            }
        }
    }
    return state;
}

void ContinuityConstraintManager::RestoreState(Scene* scene, Polygon* poly, Vertex* v, ContinuityState state) {
    if (!scene || !poly || !v || (!state.isG1 && !state.isC1)) return;

    Edge* e1 = nullptr;
    Edge* e2 = nullptr;
    for (auto& e : poly->edges) {
        if (e->vertices.first.get() == v || e->vertices.second.get() == v) {
            if (!e1) e1 = e.get();
            else e2 = e.get();
        }
    }

    if (e1 && e2) {
        if (state.isG1) scene->AddConstraint(std::make_shared<G1ContinuityConstraint>(v, e1, e2));
        if (state.isC1) scene->AddConstraint(std::make_shared<C1ContinuityConstraint>(v, e1, e2));
        scene->TrySolveConstraints(v, nullptr);
    }
}