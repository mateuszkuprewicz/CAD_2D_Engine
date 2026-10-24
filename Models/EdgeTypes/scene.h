#pragma once
#include <vector>
#include <memory>
#include "polygon.h"
#include "constraintmanager.h"

class Scene {
public:
    Scene() = default;

    void addVertex(Polygon* polygon, std::shared_ptr<Vertex> newVertex, int edgeIdx);
    void delVertex(Polygon* polygon, Vertex* vertex, int prevEdgeIdx, int nextEdgeIdx);

    std::vector<std::unique_ptr<Polygon>> polygons;
    ConstraintManager constraintManager;
};