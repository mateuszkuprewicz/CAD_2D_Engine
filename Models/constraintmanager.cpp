#include "constraintmanager.h"
#include <typeinfo>
#include "Constraints/parallelismconstraint.h"
#include "Constraints/lengthconstraint.h"
#include "edge.h"
#include "Constraints/continuityconstraint.h"

ConstraintManager::ConstraintManager() {}

void ConstraintManager::AddConstraint(const std::shared_ptr<AbstractConstraint> &c) {
    AbstractConstraint* toDel = nullptr;
    auto vec = c->affectedVertices();

    if(!vec.empty()) {
        const auto& v = vec[0];
        auto it = Constraints.find(v);
        if(it != Constraints.end()) {
            for(const auto& compared_const : it->second) {
                if(c->isEqual(compared_const.get())) {
                    toDel = compared_const.get();
                    break;
                }
            }
        }
    }

    if(toDel != nullptr) DelConstraint(toDel);

    for(const auto& v : vec) {
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

    auto constraintsToDel = it->second;
    for (const auto& c : constraintsToDel) {
        DelConstraint(c.get());
    }
}

void ConstraintManager::DelEdgeConstraints(const Vertex* v1, const Vertex* v2) {
    auto it = Constraints.find(v1);
    if (it == Constraints.end()) return;

    auto list = it->second;
    it = Constraints.find(v2);
    list.insert(list.end(), it->second.begin(), it->second.end());

    for (const auto& c : list) {
        bool shouldDelete = false;

        if (auto* lenC = dynamic_cast<LengthConstraint*>(c.get())) {
            const Edge* e = lenC->GetEdge();
            if ((e->vertices.first.get() == v1 && e->vertices.second.get() == v2) ||
                      (e->vertices.first.get() == v2 && e->vertices.second.get() == v1)) {
                shouldDelete = true;
            }
        }
        else if (auto* parC = dynamic_cast<ParallelismConstraint*>(c.get())) {
            const Edge* e1 = parC->GetEdge1();
            const Edge* e2 = parC->GetEdge2();
            bool matchesE1 = ((e1->vertices.first.get() == v1 && e1->vertices.second.get() == v2) ||
                                    (e1->vertices.first.get() == v2 && e1->vertices.second.get() == v1));
            bool matchesE2 = ((e2->vertices.first.get() == v1 && e2->vertices.second.get() == v2) ||
                                    (e2->vertices.first.get() == v2 && e2->vertices.second.get() == v1));
            if (matchesE1 || matchesE2) shouldDelete = true;
        }
        else if (auto* contC = dynamic_cast<ContinuityConstraint*>(c.get())) {
            const Edge* e1 = contC->getEdge1();
            const Edge* e2 = contC->getEdge2();
            bool matchesE1 = ((e1->vertices.first.get() == v1 && e1->vertices.second.get() == v2) ||
                                    (e1->vertices.first.get() == v2 && e1->vertices.second.get() == v1));
            bool matchesE2 = ((e2->vertices.first.get() == v1 && e2->vertices.second.get() == v2) ||
                                    (e2->vertices.first.get() == v2 && e2->vertices.second.get() == v1));
            if (matchesE1 || matchesE2) shouldDelete = true;
        }
        if (shouldDelete) DelConstraint(c.get());
    }
}

void ConstraintManager::DelContinuityConstraint(const Vertex* v) {
    auto it = Constraints.find(v);
    if (it == Constraints.end()) return;

    auto list = it->second;
    for (const auto& c : list) {
        if (auto* cont = dynamic_cast<ContinuityConstraint*>(c.get())) {
            if (cont->getVertex() == v) {
                DelConstraint(c.get());
            }
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

    auto it = Constraints.find(v1);
    if (it != Constraints.end()) {
        for (const auto& c : it->second) {
            if (auto* parConstraint = dynamic_cast<ParallelismConstraint*>(c.get())) {
                const Edge* c_e1 = parConstraint->GetEdge1();
                const Edge* c_e2 = parConstraint->GetEdge2();

                if ((c_e1 == e1 && c_e2 == e2) || (c_e1 == e2 && c_e2 == e1)) {
                    return c.get();
                }
            }
        }
    }
    return nullptr;
}