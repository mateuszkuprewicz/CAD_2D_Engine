#include "scene.h"
#include <queue>
#include <unordered_set>
#include "../consts.h"
#include "vertex.h"
#include "edge.h"
#include <tuple>
#include "continuityconstraintmanager.h"
#include "Constraints/continuityconstraint.h"
#include "Constraints/lengthconstraint.h"
#include "Constraints/parallelismconstraint.h"


Scene::Scene() {
    Polygons.push_back(std::make_unique<Polygon>(
        std::initializer_list<QPoint>{
            QPoint(50, 250), QPoint(200, 150), QPoint(350, 250),
            QPoint(250, 400), QPoint(100, 450)
        }
        ));

    Polygons.push_back(std::make_unique<Polygon>(
        std::initializer_list<QPoint>{
            QPoint(450, 250), QPoint(550, 150), QPoint(750, 200),
            QPoint(800, 350), QPoint(650, 500), QPoint(400, 400)
        }
        ));

    Edge* p1_e0 = Polygons[0]->edges[0].get();
    Edge* p1_e1 = Polygons[0]->edges[1].get();
    Edge* p1_e2 = Polygons[0]->edges[2].get();

    p1_e0->bezier = std::make_unique<BezierCurve>(
        p1_e0->vertices.first->point,
        p1_e0->vertices.second->point
        );

    Vertex* sharedVertexP1 = p1_e0->vertices.second.get();
    AddConstraint(std::make_shared<G1ContinuityConstraint>(sharedVertexP1, p1_e0, p1_e1));
    TrySolveConstraints(sharedVertexP1);

    AddConstraint(std::make_shared<LengthConstraint>(p1_e2, 150.0f));
    TrySolveConstraints(p1_e2->vertices.first.get());

    Edge* p2_e0 = Polygons[1]->edges[0].get();
    Edge* p2_e1 = Polygons[1]->edges[1].get();
    Edge* p2_e2 = Polygons[1]->edges[2].get();
    Edge* p2_e4 = Polygons[1]->edges[4].get();

    Vertex* sharedVertexP2 = p2_e0->vertices.second.get();
    p2_e0->bezier = std::make_unique<BezierCurve>(
        p2_e0->vertices.first->point,
        p2_e0->vertices.second->point);
    AddConstraint(std::make_shared<C1ContinuityConstraint>(sharedVertexP2, p2_e0, p2_e1));
    TrySolveConstraints(sharedVertexP2);

    AddConstraint(std::make_shared<ParallelismConstraint>(p2_e2, p2_e4));
    TrySolveConstraints(p2_e2->vertices.first.get());
}

void Scene::AddConstraint(std::shared_ptr<AbstractConstraint> c) {
    lastAddedConstraint = c.get();
    constraintManager.AddConstraint(std::move(c));
}
void Scene::DelConstraint(const AbstractConstraint *c) {
    constraintManager.DelConstraint(c);
}

void Scene::addVertex(Polygon* polygon, std::shared_ptr<Vertex> newVertex, int edgeIdx) {
    if (!polygon || edgeIdx < 0 || edgeIdx >= polygon->edges.size()) return;

    Vertex* v1 = polygon->edges[edgeIdx]->vertices.first.get();
    Vertex* v2 = polygon->edges[edgeIdx]->vertices.second.get();

    auto state1 = ContinuityConstraintManager::SaveState(&constraintManager, v1);
    auto state2 = ContinuityConstraintManager::SaveState(&constraintManager, v2);

    constraintManager.DelEdgeConstraints(v1, v2);
    polygon->addVertex(newVertex, edgeIdx);

    ContinuityConstraintManager::RestoreState(this, polygon, v1, state1);
    ContinuityConstraintManager::RestoreState(this, polygon, v2, state2);
}

void Scene::delVertex(Polygon* polygon, Vertex* vertex, int prevEdgeIdx, int nextEdgeIdx) {
    if (!polygon || !vertex) return;
    constraintManager.DelConstraintsForVertex(vertex);

    polygon->delVertex(vertex, prevEdgeIdx, nextEdgeIdx);
}

void Scene::SolveConstraints(Vertex* theImmovableVertex) {
    if(theImmovableVertex == nullptr) return;
    std::queue<const Vertex *> q;
    std::unordered_set<const Vertex*> queued_vertices;
    q.push(theImmovableVertex);
    queued_vertices.insert(theImmovableVertex);
    int global_iterations = 0;
    while(!q.empty() && global_iterations < ConstraintError::maxGlobalIterations) {
        auto v = q.front();
        q.pop();
        queued_vertices.erase(v);
        for(const auto& constraint : constraintManager.Constraints[v]) {
            if(constraint->getError() >= constraint->maxError())
            {
                constraint->Fix(theImmovableVertex);
                global_iterations++;
                for(const auto& neighbouring_vertex : constraint->affectedVertices()) {
                    if(queued_vertices.find(neighbouring_vertex) == queued_vertices.end()) {
                        q.push(neighbouring_vertex);
                        queued_vertices.insert(neighbouring_vertex);
                    }
                }
            }
        }
    }
}

bool Scene::TrySolveConstraints(Vertex* priorityVertex, Vertex* priorityVertex2) {
    if(priorityVertex == nullptr) return false;

    std::unordered_map<Edge *, Edge> previous_state;
    for(const auto &poly : Polygons)
    {
        for(const auto& edge : poly->edges) {
            previous_state.insert_or_assign(
                edge.get(),
                *edge
                );
        }
    }

    std::unordered_map<AbstractConstraint*, int> fix_count_map;
    std::unordered_set<const Vertex *> queued_vertices;
    std::queue<const Vertex *> q;

    q.push(priorityVertex);
    queued_vertices.insert(priorityVertex);

    if(priorityVertex2 != nullptr) {
        q.push(priorityVertex2);
        queued_vertices.insert(priorityVertex2);
    }

    bool success = true;
    while(!q.empty() && success) {
        auto v = q.front();
        q.pop();
        queued_vertices.erase(v);

        for(const auto& constraint : constraintManager.Constraints[v]) {
            if(constraint->getError() >= constraint->maxError())
            {
                constraint->Fix(nullptr);
                if(fix_count_map.find(constraint.get()) == fix_count_map.end())
                    fix_count_map[constraint.get()] = 1;
                else {
                    if(fix_count_map[constraint.get()] >= ConstraintError::maxIterationsForConstraint) {
                        success = false;
                        break;
                    }
                    ++fix_count_map[constraint.get()];
                }
                for(const auto& neighbouring_vertex : constraint->affectedVertices()) {
                    if(queued_vertices.find(neighbouring_vertex) == queued_vertices.end()) {
                        q.push(neighbouring_vertex);
                        queued_vertices.insert(neighbouring_vertex);
                    }
                }
            }
        }
    }

    if (!success) {
        DelConstraint(lastAddedConstraint);

        for (const auto& [edgePtr, edge] : previous_state) {

            edgePtr->vertices.first->point = edge.vertices.first->point;
            edgePtr->vertices.second->point = edge.vertices.second->point;
            if(edge.isArc()) {
                edgePtr->arc->radius = edge.arc->radius;
                edgePtr->arc->isMajorArc = edge.arc->isMajorArc;
            }
            if(edge.isBezier()) {
                edgePtr->bezier->controlP1->point = edge.bezier->controlP1->point;
                edgePtr->bezier->controlP2->point = edge.bezier->controlP2->point;
            }
        }
    }

    lastAddedConstraint = nullptr;

    return success;
}