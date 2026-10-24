#include "edge.h"
#include "vertex.h"
#include <cmath>

Edge::Edge(std::shared_ptr<Vertex> v1, std::shared_ptr<Vertex>v2) {
    vertices.first = v1;
    vertices.second = v2;

    eType = EdgeType::normal;
}

float Edge::GetLength() const {
    return std::hypot(vertices.first->point.x() - vertices.second->point.x(), vertices.first->point.y() - vertices.second->point.y());
}