#pragma once
#include <vector>

class Vertex;

class AbstractConstraint
{
public:
    AbstractConstraint();
    virtual void Fix(const Vertex *theImmovableVertex) const = 0;
    virtual std::vector<const Vertex *> affectedVertices() const = 0;
private:
    virtual float getError() = 0;
};