#pragma once
#include <utility>
#include <optional>
#include <memory>

#include "EdgeTypes/edge_type.h"

class Vertex;

class Edge
{
public:
    Edge(std::shared_ptr<Vertex> v1, std::shared_ptr<Vertex> v2);
    std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>> vertexes;
    std::optional<std::unique_ptr<EdgeType>> eType;
};

