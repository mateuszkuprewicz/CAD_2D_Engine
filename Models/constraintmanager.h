#pragma once
#include <unordered_map>
#include <memory>
#include <vector>

class Vertex;
class AbstractConstraint;

class ConstraintManager
{
public:
    ConstraintManager();
    void AddConstraint(const std::shared_ptr<AbstractConstraint> &);
    void DelConstraint(const AbstractConstraint *);
    void SolveConstraints(const Vertex* theImmovableVertex);
    void DelConstraintsForVertex(const Vertex *);
    void DelEdgeConstraints(const Vertex* v1, const Vertex* v2);
    const std::unordered_map<const Vertex *, std::vector<std::shared_ptr<AbstractConstraint>>>& Constraints();
private:
    std::unordered_map<const Vertex *, std::vector<std::shared_ptr<AbstractConstraint>>> constraints;
    bool areVertexSetsEqual(const std::vector<const Vertex*> vec1, const std::vector<const Vertex*> vec2);
};