#pragma once
#include <utility>
#include <memory>
#include "edgetypes.h"


class Vertex; class BezierCurve;

class Edge
{
public:
    Edge(std::shared_ptr<Vertex> v1, std::shared_ptr<Vertex> v2);
    std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>> vertices;
    float GetLength() const;
    std::unique_ptr<BezierCurve> bezier;
    bool isBezier() const { return bezier != nullptr; }
};