#pragma once

class Vertex;

class AbstractConstraint
{
public:
    AbstractConstraint();
    virtual void Fix(Vertex *theImmovableVertex) const = 0;
private:
    virtual float getError() = 0;
};
