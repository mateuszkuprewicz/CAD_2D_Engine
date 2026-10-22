#pragma once
#include "abstractconstraint.h"

class Edge;

class LengthConstraint : public AbstractConstraint
{
public:
    LengthConstraint(Edge* e, float l) : edge(e), length(l) {}
    void Fix(Vertex *theImmovableVertex) const override;
    const Edge* GetEdge() const;
    float getError() override;
private:
    Edge* edge;
    float length;
};

