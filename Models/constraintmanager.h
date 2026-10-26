#pragma once
#include <unordered_map>
#include <memory>
#include <vector>
#include "Constraints/abstractconstraint.h"
#include <algorithm>

class Vertex; class Edge;

class ConstraintManager
{
public:
    ConstraintManager();
    void AddConstraint(const std::shared_ptr<AbstractConstraint> &);
    void DelConstraint(const AbstractConstraint *);
    void DelConstraintsForVertex(const Vertex *);
    void DelEdgeConstraints(const Vertex* v1, const Vertex* v2);
    void DelContinuityConstraint(const Vertex* v);


    template<typename T>
    bool HasEdgeConstraint(const Vertex* v1, const Vertex* v2) const {
        auto it = Constraints.find(v1);
        if (it != Constraints.end()) {
            for (const auto& c : it->second) {
                if (dynamic_cast<T*>(c.get())) {
                    auto affected = c->affectedVertices();
                    if (std::find(affected.begin(), affected.end(), v2) != affected.end()) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    AbstractConstraint* GetLengthConstraint(const Vertex* v1, const Vertex* v2) const;
    AbstractConstraint* GetParallelConstraint(const Edge* e1, const Edge* e2) const;
    std::unordered_map<const Vertex *, std::vector<std::shared_ptr<AbstractConstraint>>> Constraints;
private:
    bool areVertexSetsEqual(const std::vector<const Vertex*> vec1, const std::vector<const Vertex*> vec2);
};