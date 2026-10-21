#pragma once
#include <vector>
class Polygon;
class Vertex;


class ConstraintSolver
{
public:
    ConstraintSolver() = default;
    void Solve(const std::vector<Polygon*> polygons, Vertex *theImmovableVertex);
};