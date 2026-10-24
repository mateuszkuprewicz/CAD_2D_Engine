#include "constraintmanager.h"
#include <typeinfo>
#include "Constraints/parallelismconstraint.h"
#include "Constraints/lengthconstraint.h"
#include "edge.h"

ConstraintManager::ConstraintManager() {}

void ConstraintManager::AddConstraint(const std::shared_ptr<AbstractConstraint> &c) {
    AbstractConstraint* toDel = nullptr;
    auto vec = c->affectedVertices();
    if(vec.size() > 0) {
        const auto& v = vec[0];
        const auto& compared_const = Constraints.find(v);
        for(const auto& compared_const : Constraints[v]) {
            if(typeid(*c) == typeid(*compared_const) && areVertexSetsEqual(vec, compared_const->affectedVertices())) {
                toDel = compared_const.get();
                break;
            }
        }
    }

    if(toDel != nullptr) DelConstraint(toDel);

    for(const auto& v : vec)
    {
        Constraints[v].push_back(c);
    }
}

bool ConstraintManager::areVertexSetsEqual(std::vector<const Vertex*> vec1, std::vector<const Vertex*> vec2) {
    if (vec1.size() != vec2.size()) return false;
    std::sort(vec1.begin(), vec1.end());
    std::sort(vec2.begin(), vec2.end());
    return vec1 == vec2;
}

void ConstraintManager::DelConstraint(const AbstractConstraint* c) {
    if (!c) return;

    for (const auto& v : c->affectedVertices()) {
        auto it = Constraints.find(v);
        if (it != Constraints.end()) {
            auto& vec = it->second;
            auto found = std::find_if(vec.begin(), vec.end(), [c](const auto& ptr) {
                return ptr.get() == c;
            });

            if (found != vec.end()) {
                vec.erase(found);
            }
        }
    }
}

void ConstraintManager::DelConstraintsForVertex(const Vertex* v) {
    auto it = Constraints.find(v);
    if (it == Constraints.end()) return;

    // Copy list to prevent iterator invalidation during erasure
    auto constraintsToDel = it->second;
    for (const auto& c : constraintsToDel) {
        DelConstraint(c.get());
    }
}

void ConstraintManager::DelEdgeConstraints(const Vertex* v1, const Vertex* v2) {
    auto it = Constraints.find(v1);
    if (it == Constraints.end()) return;

    auto list = it->second;
    for (const auto& c : list) {
        auto affected = c->affectedVertices();
        bool affectsV2 = (std::find(affected.begin(), affected.end(), v2) != affected.end());

        if (affectsV2) {
            DelConstraint(c.get());
        }
    }
}

AbstractConstraint* ConstraintManager::GetLengthConstraint(const Vertex* v1, const Vertex* v2) const {
    auto it = Constraints.find(v1);
    if (it != Constraints.end()) {
        for (const auto& c : it->second) {
            if (dynamic_cast<LengthConstraint*>(c.get())) {
                auto affected = c->affectedVertices();
                if (std::find(affected.begin(), affected.end(), v2) != affected.end()) {
                    return c.get();
                }
            }
        }
    }
    return nullptr;
}

AbstractConstraint* ConstraintManager::GetParallelConstraint(const Edge* e1, const Edge* e2) const {
    if (!e1 || !e2) return nullptr;
    auto v1 = e1->vertices.first.get();
    auto v3 = e2->vertices.first.get();

    auto it = Constraints.find(v1);
    if (it != Constraints.end()) {
        for (const auto& c : it->second) {
            if (dynamic_cast<ParallelismConstraint*>(c.get())) {
                auto affected = c->affectedVertices();
                if (std::find(affected.begin(), affected.end(), v3) != affected.end()) {
                    return c.get();
                }
            }
        }
    }
    return nullptr;
}