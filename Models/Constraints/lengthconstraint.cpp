#include "lengthconstraint.h"
#include "../edge.h"
#include "../vertex.h"
#include <cmath>
#include "../../consts.h"

float LengthConstraint::getError() {
    return std::abs(length - edge->GetLength());
}

float LengthConstraint::maxError() {
    return ConstraintError::length;
}

void LengthConstraint::Fix(const Vertex *theImmovableVertex) const {
    auto v1 = edge->vertices.first.get();
    auto v2 = edge->vertices.second.get();

    double currentLength = edge->GetLength();

    double diff = currentLength - length;

    double dx = v2->point.x() - v1->point.x();
    double dy = v2->point.y() - v1->point.y();

    double proportion = diff / currentLength;
    double offsetX = dx * proportion;
    double offsetY = dy * proportion;

    if (v1 == theImmovableVertex) {
        v2->point.rx() -= offsetX;
        v2->point.ry() -= offsetY;
    }
    else if (v2 == theImmovableVertex) {
        v1->point.rx() += offsetX;
        v1->point.ry() += offsetY;
    }
    else {
        v1->point.rx() += offsetX / 2.0;
        v1->point.ry() += offsetY / 2.0;

        v2->point.rx() -= offsetX / 2.0;
        v2->point.ry() -= offsetY / 2.0;
    }
}

const Edge* LengthConstraint::GetEdge() const {
    return edge;
}

std::vector<const Vertex *> LengthConstraint::affectedVertices() const {
    std::vector<const Vertex *> result;
    result.push_back(edge->vertices.first.get());
    result.push_back(edge->vertices.second.get());
    return result;
}