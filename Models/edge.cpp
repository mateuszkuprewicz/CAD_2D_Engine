#include "edge.h"
#include "vertex.h"

Edge::Edge(std::shared_ptr<Vertex> v1, std::shared_ptr<Vertex>v2) {
    vertexes.first = v1;
    vertexes.second = v2;

    eType = std::nullopt;
}

