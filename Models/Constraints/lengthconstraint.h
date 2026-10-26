#pragma once
#include "abstractconstraint.h"

class Edge;

class LengthConstraint : public AbstractConstraint
{
public:
    LengthConstraint(Edge* e, float l) : edge(e), length(l) {}
    void Fix(const Vertex *theImmovableVertex) const override;
    const Edge* GetEdge() const;
    float getError() override;
    float maxError() override;
    std::vector<const Vertex *> affectedVertices() const override;
    bool isEqual(const AbstractConstraint* other) const override;
private:
    Edge* edge;
    float length;
};

