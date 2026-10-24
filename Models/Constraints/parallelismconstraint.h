#pragma once
#include <memory>
#include <optional>
#include "abstractconstraint.h"
#include <vector>

class Vertex; class Edge;

class ParallelismConstraint : public AbstractConstraint
{
public:
    static std::optional<std::unique_ptr<ParallelismConstraint>> Create(Edge* e1, Edge* e2);
    void Fix(const Vertex *theImmovableVertex) const override;
    float getError() override;
    std::vector<const Vertex *> affectedVertices() const override;
    const Edge* GetEdge1() const;
    const Edge* GetEdge2() const;
private:
    ParallelismConstraint(Edge* e1, Edge* e2);
    Edge* parallel_edges[2];
};
