#pragma once
#include <vector>

class Vertex;

class AbstractConstraint
{
public:
    AbstractConstraint();
    virtual void Fix(const Vertex *theImmovableVertex) const = 0;
    virtual std::vector<const Vertex *> affectedVertices() const = 0;
    virtual float getError() = 0;
    virtual float maxError() = 0;
};