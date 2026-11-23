#include "continuityconstraint.h"
#include "../edge.h"
#include "../vertex.h"
#include "../../consts.h"
#include <cmath>

ContinuityConstraint::ContinuityConstraint(Vertex* sharedVertex, Edge* edge1, Edge* edge2)
    : sharedVertex(sharedVertex), edge1(edge1), edge2(edge2) {}

bool ContinuityConstraint::isEqual(const AbstractConstraint* other) const {
    if (const auto* o = dynamic_cast<const ContinuityConstraint*>(other)) {
        return o->sharedVertex == sharedVertex;
    }
    return false;
}

std::vector<const Vertex*> ContinuityConstraint::affectedVertices() const {
    std::vector<const Vertex*> result = { sharedVertex };
    if (edge1->isBezier()) {
        if (edge1->vertices.first.get() == sharedVertex) result.push_back(edge1->bezier->controlP1.get());
        else result.push_back(edge1->bezier->controlP2.get());
    } else {
        result.push_back(edge1->vertices.first.get() == sharedVertex ? edge1->vertices.second.get() : edge1->vertices.first.get());
    }

    if (edge2->isBezier()) {
        if (edge2->vertices.first.get() == sharedVertex) result.push_back(edge2->bezier->controlP1.get());
        else result.push_back(edge2->bezier->controlP2.get());
    } else {
        result.push_back(edge2->vertices.first.get() == sharedVertex ? edge2->vertices.second.get() : edge2->vertices.first.get());
    }
    return result;
}

void ContinuityConstraint::getTangent(Edge* edge, Vertex* shared, double& tx, double& ty) const {
    if (edge->isBezier()) {
        Vertex* p = (edge->vertices.first.get() == shared) ? edge->bezier->controlP1.get() : edge->bezier->controlP2.get();
        tx = p->point.x() - shared->point.x();
        ty = p->point.y() - shared->point.y();
    } else if (edge->isArc()) {
        bool isFirst = (edge->vertices.first.get() == shared);
        Vertex* pOther = isFirst ? edge->vertices.second.get() : edge->vertices.first.get();

        double chordX = pOther->point.x() - shared->point.x();
        double chordY = pOther->point.y() - shared->point.y();
        double chordLen = std::hypot(chordX, chordY);

        if (chordLen < 0.001) { tx = 0; ty = 0; return; }

        double R = edge->arc->radius;

        // Protect against invalid arcs
        if (std::abs(R) < chordLen / 2.0) {
            R = (R >= 0 ? 1.0 : -1.0) * (chordLen / 2.0);
        }

        // Derive rotation angle directly mirroring setArcRadius math
        double sin_alpha = -chordLen / (2.0 * R);
        if (!isFirst) {
            sin_alpha = -sin_alpha;
        }

        // Clamp to [-1, 1] to prevent NaN in sqrt
        sin_alpha = std::max(-1.0, std::min(1.0, sin_alpha));
        double cos_alpha = std::sqrt(1.0 - sin_alpha * sin_alpha);

        // Apply major arc flag (this was completely missing in your code)
        if (edge->arc->isMajorArc) {
            cos_alpha = -cos_alpha;
        }

        double ux = chordX / chordLen;
        double uy = chordY / chordLen;

        // Rotate vector pointing away from shared vertex
        tx = ux * cos_alpha - uy * sin_alpha;
        ty = ux * sin_alpha + uy * cos_alpha;
    } else {
        Vertex* p = (edge->vertices.first.get() == shared) ? edge->vertices.second.get() : edge->vertices.first.get();
        tx = p->point.x() - shared->point.x();
        ty = p->point.y() - shared->point.y();
    }
    double len = std::hypot(tx, ty);
    if (len > 0.001) { tx /= len; ty /= len; }
}

float G1ContinuityConstraint::maxError() { return 0.02; }

float G1ContinuityConstraint::getError() {
    double tx1 = 0, ty1 = 0, tx2 = 0, ty2 = 0;
    getTangent(edge1, sharedVertex, tx1, ty1);
    getTangent(edge2, sharedVertex, tx2, ty2);
    return std::hypot(tx1 + tx2, ty1 + ty2);
}

void G1ContinuityConstraint::Fix(const Vertex* theImmovableVertex) const {
    auto setArcRadius = [](Edge* arcEdge, Vertex* shared, double tgtX, double tgtY) {
        bool isFirst = (arcEdge->vertices.first.get() == shared);
        Vertex* arcEnd = isFirst ? arcEdge->vertices.second.get() : arcEdge->vertices.first.get();
        double chordX = arcEnd->point.x() - shared->point.x();
        double chordY = arcEnd->point.y() - shared->point.y();
        double chordLenSq = chordX * chordX + chordY * chordY;

        double cross = tgtX * chordY - tgtY * chordX;

        if (!isFirst) {
            cross = -cross;
        }

        double dot = tgtX * chordX + tgtY * chordY;
        arcEdge->arc->isMajorArc = (dot < 0.0);

        if (std::abs(cross) < 0.0001) {
            arcEdge->arc->radius = (cross >= 0 ? 1.0 : -1.0) * 1000000.0;
        } else {
            arcEdge->arc->radius = chordLenSq / (2.0 * cross);
        }
    };

    if (!edge1->isArc() && !edge2->isArc()) {
        Vertex* p1 = edge1->isBezier() ? (edge1->vertices.first.get() == sharedVertex ? edge1->bezier->controlP1.get() : edge1->bezier->controlP2.get()) : (edge1->vertices.first.get() == sharedVertex ? edge1->vertices.second.get() : edge1->vertices.first.get());
        Vertex* p2 = edge2->isBezier() ? (edge2->vertices.first.get() == sharedVertex ? edge2->bezier->controlP1.get() : edge2->bezier->controlP2.get()) : (edge2->vertices.first.get() == sharedVertex ? edge2->vertices.second.get() : edge2->vertices.first.get());

        double dx1 = p1->point.x() - sharedVertex->point.x();
        double dy1 = p1->point.y() - sharedVertex->point.y();
        double dx2 = p2->point.x() - sharedVertex->point.x();
        double dy2 = p2->point.y() - sharedVertex->point.y();
        double len1 = std::hypot(dx1, dy1);
        double len2 = std::hypot(dx2, dy2);
        if (len1 < 0.001 || len2 < 0.001) return;

        double nx1 = dx1 / len1, ny1 = dy1 / len1;
        double nx2 = dx2 / len2, ny2 = dy2 / len2;

        if (p1 == theImmovableVertex) {
            p2->point.setX(sharedVertex->point.x() - nx1 * len2);
            p2->point.setY(sharedVertex->point.y() - ny1 * len2);
        } else if (p2 == theImmovableVertex) {
            p1->point.setX(sharedVertex->point.x() - nx2 * len1);
            p1->point.setY(sharedVertex->point.y() - ny2 * len1);
        } else {
            if (edge1->isBezier() && !edge2->isBezier()) {
                p1->point.setX(sharedVertex->point.x() - nx2 * len1);
                p1->point.setY(sharedVertex->point.y() - ny2 * len1);
            } else if (edge2->isBezier() && !edge1->isBezier()) {
                p2->point.setX(sharedVertex->point.x() - nx1 * len2);
                p2->point.setY(sharedVertex->point.y() - ny1 * len2);
            } else {
                p2->point.setX(sharedVertex->point.x() - nx1 * len2);
                p2->point.setY(sharedVertex->point.y() - ny1 * len2);
            }
        }
    } else if (edge1->isArc() && edge2->isArc()) {
        double t1x = 0, t1y = 0; getTangent(edge1, sharedVertex, t1x, t1y);
        double t2x = 0, t2y = 0; getTangent(edge2, sharedVertex, t2x, t2y);

        double tgtX = t1x - t2x;
        double tgtY = t1y - t2y;
        double len = std::hypot(tgtX, tgtY);
        if (len > 0.001) { tgtX /= len; tgtY /= len; }
        else { tgtX = t1x; tgtY = t1y; }

        setArcRadius(edge1, sharedVertex, tgtX, tgtY);
        setArcRadius(edge2, sharedVertex, -tgtX, -tgtY);
    } else {
        Edge* arcEdge = edge1->isArc() ? edge1 : edge2;
        Edge* otherEdge = edge1->isArc() ? edge2 : edge1;

        if (otherEdge->isBezier()) {
            Vertex* bezP = (otherEdge->vertices.first.get() == sharedVertex) ? otherEdge->bezier->controlP1.get() : otherEdge->bezier->controlP2.get();

            if (bezP == theImmovableVertex) {
                double tx = 0, ty = 0; getTangent(otherEdge, sharedVertex, tx, ty);
                setArcRadius(arcEdge, sharedVertex, -tx, -ty);
            } else {
                double taX = 0, taY = 0; getTangent(arcEdge, sharedVertex, taX, taY);
                double tbX = 0, tbY = 0; getTangent(otherEdge, sharedVertex, tbX, tbY);

                double tgtX = taX - tbX;
                double tgtY = taY - tbY;
                double len = std::hypot(tgtX, tgtY);
                if (len > 0.001) { tgtX /= len; tgtY /= len; }
                else { tgtX = taX; tgtY = taY; }

                setArcRadius(arcEdge, sharedVertex, tgtX, tgtY);

                double dist = std::hypot(bezP->point.x() - sharedVertex->point.x(), bezP->point.y() - sharedVertex->point.y());
                if (dist < 1.0) dist = 50.0;
                bezP->point.setX(sharedVertex->point.x() - tgtX * dist);
                bezP->point.setY(sharedVertex->point.y() - tgtY * dist);
            }
        } else {
            double tx = 0, ty = 0; getTangent(otherEdge, sharedVertex, tx, ty);
            setArcRadius(arcEdge, sharedVertex, -tx, -ty);
        }
    }
}

float C1ContinuityConstraint::maxError() { return 0.1f; }

float C1ContinuityConstraint::getError() {
    if (edge1->isArc() || edge2->isArc()) return 0.0f;

    Vertex* p1 = edge1->isBezier() ? (edge1->vertices.first.get() == sharedVertex ? edge1->bezier->controlP1.get() : edge1->bezier->controlP2.get()) : (edge1->vertices.first.get() == sharedVertex ? edge1->vertices.second.get() : edge1->vertices.first.get());
    Vertex* p2 = edge2->isBezier() ? (edge2->vertices.first.get() == sharedVertex ? edge2->bezier->controlP1.get() : edge2->bezier->controlP2.get()) : (edge2->vertices.first.get() == sharedVertex ? edge2->vertices.second.get() : edge2->vertices.first.get());

    double dx1 = p1->point.x() - sharedVertex->point.x();
    double dy1 = p1->point.y() - sharedVertex->point.y();
    if (edge1->isBezier())
    {
        dx1*=3;
        dy1*=3;
    }
    double dx2 = p2->point.x() - sharedVertex->point.x();
    double dy2 = p2->point.y() - sharedVertex->point.y();
    if (edge2->isBezier())
    {
        dx2*=3;
        dy2*=3;
    }

    return std::hypot(dx1 + dx2, dy1 + dy2);
}

void C1ContinuityConstraint::Fix(const Vertex* theImmovableVertex) const {
    if (edge1->isArc() || edge2->isArc()) return;

    Vertex* p1 = edge1->isBezier() ? (edge1->vertices.first.get() == sharedVertex ? edge1->bezier->controlP1.get() : edge1->bezier->controlP2.get()) : (edge1->vertices.first.get() == sharedVertex ? edge1->vertices.second.get() : edge1->vertices.first.get());
    Vertex* p2 = edge2->isBezier() ? (edge2->vertices.first.get() == sharedVertex ? edge2->bezier->controlP1.get() : edge2->bezier->controlP2.get()) : (edge2->vertices.first.get() == sharedVertex ? edge2->vertices.second.get() : edge2->vertices.first.get());

    double dx1 = p1->point.x() - sharedVertex->point.x();
    double dy1 = p1->point.y() - sharedVertex->point.y();
    double dx2 = p2->point.x() - sharedVertex->point.x();
    double dy2 = p2->point.y() - sharedVertex->point.y();
    if (edge2->isBezier() && !edge1->isBezier()) {
        dx1/=3;
        dy1/=3;
        dx2*=3;
        dy2*=3;
    }
    if (edge1->isBezier() && !edge2->isBezier()) {
        dx2/=3;
        dy2/=3;
        dx1*=3;
        dy1*=3;
    }


    if (p1 == theImmovableVertex) {
        p2->point.setX(sharedVertex->point.x() - dx1);
        p2->point.setY(sharedVertex->point.y() - dy1);
    } else if (p2 == theImmovableVertex) {
        p1->point.setX(sharedVertex->point.x() - dx2);
        p1->point.setY(sharedVertex->point.y() - dy2);
    } else {
        if (edge1->isBezier() && !edge2->isBezier()) {
            p1->point.setX(sharedVertex->point.x() - dx2);
            p1->point.setY(sharedVertex->point.y() - dy2);
        } else if (edge2->isBezier() && !edge1->isBezier()) {
            p2->point.setX(sharedVertex->point.x() - dx1);
            p2->point.setY(sharedVertex->point.y() - dy1);
        } else {
            double avgX = (dx1 - dx2) / 2.0;
            double avgY = (dy1 - dy2) / 2.0;
            p1->point.setX(sharedVertex->point.x() + avgX);
            p1->point.setY(sharedVertex->point.y() + avgY);
            p2->point.setX(sharedVertex->point.x() - avgX);
            p2->point.setY(sharedVertex->point.y() - avgY);
        }
    }
}