#pragma once
#include <memory>
#include <optional>
#include "abstractconstraint.h"

class Vertex; class Edge;

class ParallelismConstraint : public AbstractConstraint
{
public:
    static std::optional<std::unique_ptr<ParallelismConstraint>> Create(Edge* e1, Edge* e2);
    void Fix(Vertex *theImmovableVertex) const override;
    float getError() override;
    const Edge* GetEdge1() const { return parallel_edges[0]; }
    const Edge* GetEdge2() const { return parallel_edges[1]; }
private:
    ParallelismConstraint(Edge* e1, Edge* e2);
    Edge* parallel_edges[2];
};
