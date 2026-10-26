#pragma once
#include "abstractconstraint.h"

class Edge;
class Vertex;

class ContinuityConstraint : public AbstractConstraint {
public:
    ContinuityConstraint(Vertex* sharedVertex, Edge* edge1, Edge* edge2);
    bool isEqual(const AbstractConstraint* other) const override;
    std::vector<const Vertex*> affectedVertices() const override;

    Vertex* getVertex() const { return sharedVertex; }
    Edge* getEdge1() const { return edge1; }
    Edge* getEdge2() const { return edge2; }

protected:
    Vertex* sharedVertex;
    Edge* edge1;
    Edge* edge2;

    void getTangent(Edge* edge, Vertex* shared, double& tx, double& ty) const;
};

class G1ContinuityConstraint : public ContinuityConstraint {
public:
    G1ContinuityConstraint(Vertex* sharedVertex, Edge* edge1, Edge* edge2)
        : ContinuityConstraint(sharedVertex, edge1, edge2) {}
    void Fix(const Vertex* theImmovableVertex) const override;
    float getError() override;
    float maxError() override;
};

class C1ContinuityConstraint : public ContinuityConstraint {
public:
    C1ContinuityConstraint(Vertex* sharedVertex, Edge* edge1, Edge* edge2)
        : ContinuityConstraint(sharedVertex, edge1, edge2) {}
    void Fix(const Vertex* theImmovableVertex) const override;
    float getError() override;
    float maxError() override;
};