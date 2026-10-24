#pragma once
#include <utility>
#include <memory>


class Vertex;

enum class EdgeType {normal, besier_curve, circle_cuve};

class Edge
{
public:
    Edge(std::shared_ptr<Vertex> v1, std::shared_ptr<Vertex> v2);
    std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>> vertices;
    float GetLength() const;
    EdgeType eType;
};

