#include "scene.h"
#include <queue>
#include <unordered_set>
#include "../consts.h"
#include "vertex.h"
#include "edge.h"
#include <tuple>

Scene::Scene() {
    Polygons.push_back(std::make_unique<Polygon>(
        std::initializer_list<QPoint>{QPoint(100, 100), QPoint(300, 100), QPoint(300, 300), QPoint(100, 300)}
        ));
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

    constraintManager.DelEdgeConstraints(v1, v2);

    polygon->addVertex(newVertex, edgeIdx);
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

    std::unordered_map<Edge *, std::pair<Vertex, Vertex>> previous_state;
    for(const auto &poly : Polygons)
    {
        for(const auto& edge : poly->edges) {
            previous_state.insert_or_assign(
                edge.get(),
                std::make_pair(*edge->vertices.first, *edge->vertices.second)
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

        for (const auto& [edgePtr, vertices_pair] : previous_state) {

            *edgePtr->vertices.first = vertices_pair.first;
            *edgePtr->vertices.second = vertices_pair.second;
        }
    }

    lastAddedConstraint = nullptr;

    return success;
}