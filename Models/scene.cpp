#include "scene.h"

Scene::Scene() {
    Polygons.push_back(std::make_unique<Polygon>(
        std::initializer_list<QPoint>{QPoint(100, 100), QPoint(300, 100), QPoint(300, 300), QPoint(100, 300)}
        ));
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