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
        Vertex* pOther = (edge->vertices.first.get() == shared) ? edge->vertices.second.get() : edge->vertices.first.get();
        double chordX = pOther->point.x() - shared->point.x();
        double chordY = pOther->point.y() - shared->point.y();
        double chordLen = std::hypot(chordX, chordY);
        if (chordLen < 0.001) { tx = 0; ty = 0; return; }

        double R = edge->arc->radius;
        double absR = std::max(std::abs(R), chordLen / 2.0);
        double h = std::sqrt(absR * absR - (chordLen / 2.0) * (chordLen / 2.0));
        double nx = -chordY / chordLen;
        double ny = chordX / chordLen;
        double sign = (R >= 0) ? 1.0 : -1.0;

        double cx = (shared->point.x() + pOther->point.x()) / 2.0 + sign * h * nx;
        double cy = (shared->point.y() + pOther->point.y()) / 2.0 + sign * h * ny;

        double rx = shared->point.x() - cx;
        double ry = shared->point.y() - cy;

        tx = -ry;
        ty = rx;
        if (tx * chordX + ty * chordY < 0) {
            tx = -tx;
            ty = -ty;
        }
    } else {
        Vertex* p = (edge->vertices.first.get() == shared) ? edge->vertices.second.get() : edge->vertices.first.get();
        tx = p->point.x() - shared->point.x();
        ty = p->point.y() - shared->point.y();
    }
    double len = std::hypot(tx, ty);
    if (len > 0.001) { tx /= len; ty /= len; }
}

float G1ContinuityConstraint::maxError() { return 0.01f; }

float G1ContinuityConstraint::getError() {
    double tx1 = 0, ty1 = 0, tx2 = 0, ty2 = 0;
    getTangent(edge1, sharedVertex, tx1, ty1);
    getTangent(edge2, sharedVertex, tx2, ty2);
    return std::hypot(tx1 + tx2, ty1 + ty2);
}

void G1ContinuityConstraint::Fix(const Vertex* theImmovableVertex) const {
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
    } else {
        Edge* arcEdge = edge1->isArc() ? edge1 : edge2;
        Edge* otherEdge = edge1->isArc() ? edge2 : edge1;
        Vertex* arcEnd = (arcEdge->vertices.first.get() == sharedVertex) ? arcEdge->vertices.second.get() : arcEdge->vertices.first.get();
        Vertex* pOther = otherEdge->isBezier() ? (otherEdge->vertices.first.get() == sharedVertex ? otherEdge->bezier->controlP1.get() : otherEdge->bezier->controlP2.get()) : (otherEdge->vertices.first.get() == sharedVertex ? otherEdge->vertices.second.get() : otherEdge->vertices.first.get());

        if (pOther == theImmovableVertex || arcEnd != theImmovableVertex) {
            double tx = 0, ty = 0;
            getTangent(otherEdge, sharedVertex, tx, ty);
            double tgtX = -tx, tgtY = -ty;
            double chordX = arcEnd->point.x() - sharedVertex->point.x();
            double chordY = arcEnd->point.y() - sharedVertex->point.y();
            double chordLenSq = chordX * chordX + chordY * chordY;
            double cross = tgtX * chordY - tgtY * chordX;
            if (std::abs(cross) > 0.001) {
                arcEdge->arc->radius = chordLenSq / (2.0 * cross);
            }
        } else {
            double tx = 0, ty = 0;
            getTangent(arcEdge, sharedVertex, tx, ty);
            double dx = pOther->point.x() - sharedVertex->point.x();
            double dy = pOther->point.y() - sharedVertex->point.y();
            double len = std::hypot(dx, dy);
            if (len > 0.001) {
                pOther->point.setX(sharedVertex->point.x() - tx * len);
                pOther->point.setY(sharedVertex->point.y() - ty * len);
            }
        }
    }
}

float C1ContinuityConstraint::maxError() { return 0.01f; }

float C1ContinuityConstraint::getError() {
    if (!edge1->isArc() && !edge2->isArc()) {
        Vertex* p1 = edge1->isBezier() ? (edge1->vertices.first.get() == sharedVertex ? edge1->bezier->controlP1.get() : edge1->bezier->controlP2.get()) : (edge1->vertices.first.get() == sharedVertex ? edge1->vertices.second.get() : edge1->vertices.first.get());
        Vertex* p2 = edge2->isBezier() ? (edge2->vertices.first.get() == sharedVertex ? edge2->bezier->controlP1.get() : edge2->bezier->controlP2.get()) : (edge2->vertices.first.get() == sharedVertex ? edge2->vertices.second.get() : edge2->vertices.first.get());
        double dx1 = p1->point.x() - sharedVertex->point.x();
        double dy1 = p1->point.y() - sharedVertex->point.y();
        double dx2 = p2->point.x() - sharedVertex->point.x();
        double dy2 = p2->point.y() - sharedVertex->point.y();
        return std::hypot(dx1 + dx2, dy1 + dy2);
    }
    G1ContinuityConstraint g1(sharedVertex, edge1, edge2);
    return g1.getError();
}

void C1ContinuityConstraint::Fix(const Vertex* theImmovableVertex) const {
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

        if (p1 == theImmovableVertex) {
            double nx = dx1 / len1, ny = dy1 / len1;
            p2->point.setX(sharedVertex->point.x() - nx * len1);
            p2->point.setY(sharedVertex->point.y() - ny * len1);
        } else if (p2 == theImmovableVertex) {
            double nx = dx2 / len2, ny = dy2 / len2;
            p1->point.setX(sharedVertex->point.x() - nx * len2);
            p1->point.setY(sharedVertex->point.y() - ny * len2);
        } else {
            double targetLen = (len1 + len2) / 2.0;
            double nx = (dx1 / len1 - dx2 / len2) / 2.0;
            double ny = (dy1 / len1 - dy2 / len2) / 2.0;
            double nLen = std::hypot(nx, ny);
            if(nLen > 0.001) { nx /= nLen; ny /= nLen; }
            p1->point.setX(sharedVertex->point.x() + nx * targetLen);
            p1->point.setY(sharedVertex->point.y() + ny * targetLen);
            p2->point.setX(sharedVertex->point.x() - nx * targetLen);
            p2->point.setY(sharedVertex->point.y() - ny * targetLen);
        }
    } else {
        G1ContinuityConstraint g1(sharedVertex, edge1, edge2);
        g1.Fix(theImmovableVertex);
    }
}