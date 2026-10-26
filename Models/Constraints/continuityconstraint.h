#pragma once
#include "abstractconstraint.h"

enum class ContinuityLevel { G0, G1, C1 };

class AbstractContinuityConstraint : public AbstractConstraint {
public:
    AbstractContinuityConstraint(Vertex* sharedVertex) : centerVertex(sharedVertex) {}
    virtual ContinuityLevel getContinuityLevel() const = 0;
    bool isEqual(const AbstractConstraint* other) const override {
        const auto* o = dynamic_cast<const AbstractContinuityConstraint*>(other);
        if (!o) return false;

        return this->centerVertex == o->centerVertex;
    }
protected:
    Vertex* centerVertex;
};

class BezierContinuityConstraint : public AbstractContinuityConstraint {
public:
    BezierContinuityConstraint(Vertex* sharedVertex, ContinuityLevel level = ContinuityLevel::G1)
        : AbstractContinuityConstraint(sharedVertex), level(level) {}

    ContinuityLevel getContinuityLevel() const override { return level; }

    void Fix(const Vertex* theImmovableVertex) const override {

    }

    std::vector<const Vertex*> affectedVertices() const override {
        return {centerVertex}; // extend this with affected neighbouring vertices
    }

    float getError() override { return 0.0f; } // stub
    float maxError() override { return 0.001f; } // Error tolerance
private:
    ContinuityLevel level;
};