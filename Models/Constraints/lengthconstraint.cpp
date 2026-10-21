#include "lengthconstraint.h"
#include "../edge.h"
#include "../vertex.h"
#include <cmath>

float LengthConstraint::getError() {
    return std::abs(length - edge->getLength());
}

void LengthConstraint::Fix(Vertex *theImmovableVertex) const {
    const auto &v1 = edge->vertices.first.get();
    const auto &v2 = edge->vertices.second.get();

    //clean this repeatable logic
    if(v1 == theImmovableVertex) {
        float dy = v1->point.y() - v2->point.y();
        float dx = v1->point.x() - v2->point.x();

        float proportion = (edge->getLength() - length)/edge->getLength();
        v2->point.rx() += dx * proportion;
        v2->point.ry() += dy * proportion;
    }
    else if(v2 == theImmovableVertex) {
        float dy = v2->point.y() - v1->point.y();
        float dx = v2->point.x() - v1->point.x();

        float proportion = (edge->getLength() - length)/edge->getLength();
        v1->point.ry() += dy * proportion;
        v1->point.rx() += dx * proportion;
    }
    else {
        float dy = v1->point.y() - v2->point.y();
        float dx = v1->point.x() - v2->point.x();

        float proportion = (edge->getLength() - length)/edge->getLength();
        v2->point.rx() += dx * proportion/2;
        v2->point.ry() += dy * proportion/2;

        dy = v2->point.y() - v1->point.y();
        dx = v2->point.x() - v1->point.x();

        proportion = (edge->getLength() - length)/edge->getLength();
        v1->point.ry() += dy * proportion;
        v1->point.rx() += dx * proportion;

    }
}