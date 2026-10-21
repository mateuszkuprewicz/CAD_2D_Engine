#include "constraintsolver.h"
#include "../Models/polygon.h"
#include "../Models/Constraints/abstractconstraint.h"

void ConstraintSolver::Solve(const std::vector<Polygon*> polygons, Vertex *theImmovableVertex) {
    //primitive loop
    for(const auto& poly : polygons)
    {
        for(auto& constraint : poly->Constraints())
        {
            constraint->Fix(theImmovableVertex);
        }
    }
}
