#pragma once
#include <vector>
#include <memory>
#include "polygon.h"
#include "constraintmanager.h"
#include "vertex.h"


class Scene {
public:
    Scene();
    void addVertex(Polygon* polygon, std::shared_ptr<Vertex> newVertex, int edgeIdx);
    void delVertex(Polygon* polygon, Vertex* vertex, int prevEdgeIdx, int nextEdgeIdx);
    void SolveConstraints(Vertex* theImmovableVertex);
    bool TrySolveConstraints(Vertex* priorityVertex, Vertex* priorityVertex2 = nullptr);
    std::vector<std::unique_ptr<Polygon>> Polygons;
    ConstraintManager constraintManager;
    void AddConstraint(std::shared_ptr<AbstractConstraint>);
    void DelConstraint(const AbstractConstraint *);

private:
    AbstractConstraint *lastAddedConstraint = nullptr;
};