#pragma once
#include "abstractconstraint.h"
#include <vector>

class Vertex; class Edge;

class ParallelismConstraint : public AbstractConstraint
{
public:
    ParallelismConstraint(Edge* e1, Edge* e2);
    void Fix(const Vertex *theImmovableVertex) const override;
    float getError() override;
    float maxError() override;
    std::vector<const Vertex *> affectedVertices() const override;
    const Edge* GetEdge1() const;
    const Edge* GetEdge2() const;
private:
    Edge* parallel_edges[2];
};
