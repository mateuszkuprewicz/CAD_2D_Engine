#include "constraintsolver.h"
#include "../Models/polygon.h"
#include "../Models/Constraints/abstractconstraint.h"
#include <queue>

void ConstraintSolver::Solve(const std::vector<Polygon*> polygons, Vertex *theImmovableVertex) {
    std::queue<AbstractConstraint *> queueConstraints;
    //add to polygon unordered_set<Vertex *>
    //queueConstraints.push()
    //primitive loop


    for(const auto& poly : polygons) {
        if(poly->vertices.find(theImmovableVertex) != poly->vertices.end()) {

        }
    }

    for(int i = 0; i < 10; i++) {
        for(const auto& poly : polygons)
        {
            for(auto& [v, list] : poly->Constraints())
            {
                for(auto& constraint : list) {
                    constraint->Fix(theImmovableVertex);
                }
            }
        }
    }
}
