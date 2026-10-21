#pragma once
#include "abstractconstraint.h"

class Edge;

class LengthConstraint : public AbstractConstraint
{
public:
    LengthConstraint(Edge* e, float l) : edge(e), length(l) {}
    void Fix(Vertex *theImmovableVertex) const override;
private:
    float getError() override;
    Edge* edge;
    float length;
};

