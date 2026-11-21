#include "./HitTest.h"
#include "../../../Models/scene.h"
#include "../../../Models/polygon.h"
#include "../../../Models/edge.h"
#include "../../../Models/vertex.h"
#include "../../../Models/edgetypes.h"
#include "../../../consts.h"
#include <algorithm>

double HitTest::distSq(QPointF a, QPointF b) const {
    const double dx = a.x() - b.x();
    const double dy = a.y() - b.y();
    return dx * dx + dy * dy;
}

double HitTest::segmentDistSq(QPointF pt, QPointF v1, QPointF v2) const {
    const double l2 = distSq(v1, v2);
    if (l2 == 0.0) return distSq(pt, v1);

    double t = ((pt.x() - v1.x()) * (v2.x() - v1.x()) +
                (pt.y() - v1.y()) * (v2.y() - v1.y())) / l2;
    t = std::max(0.0, std::min(1.0, t));

    const QPointF proj(v1.x() + t * (v2.x() - v1.x()),
                       v1.y() + t * (v2.y() - v1.y()));

    return distSq(pt, proj);
}

Vertex* HitTest::VertexHitTest(Scene* scene,
                               QPointF pt,
                               bool includeControlPoints,
                               Polygon** outPoly,
                               size_t* outEdgeIdx) const {
    if (!scene) return nullptr;

    const double radiusSq = ClickArea::vertexRadius * ClickArea::vertexRadius;

    if (includeControlPoints) {
        for (const auto& poly : scene->Polygons) {
            for (size_t i = 0; i < poly->edges.size(); ++i) {
                const auto& edge = poly->edges[i];
                if (edge->isBezier()) {
                    if (distSq(pt, edge->bezier->controlP1->point) <= radiusSq) {
                        if (outPoly) *outPoly = poly.get();
                        if (outEdgeIdx) *outEdgeIdx = i;
                        return edge->bezier->controlP1.get();
                    }
                    if (distSq(pt, edge->bezier->controlP2->point) <= radiusSq) {
                        if (outPoly) *outPoly = poly.get();
                        if (outEdgeIdx) *outEdgeIdx = i;
                        return edge->bezier->controlP2.get();
                    }
                }
            }
        }
    }

    for (const auto& poly : scene->Polygons) {
        for (size_t i = 0; i < poly->edges.size(); ++i) {
            const auto& v = poly->edges[i]->vertices.first;
            if (distSq(pt, v->point) <= radiusSq) {
                if (outPoly) *outPoly = poly.get();
                if (outEdgeIdx) *outEdgeIdx = i;
                return v.get();
            }
        }
    }

    return nullptr;
}

Edge* HitTest::EdgeHitTest(Scene* scene,
                           QPointF pt,
                           bool ignoreNearEndpoints,
                           Polygon** outPoly,
                           size_t* outEdgeIdx) const {
    if (!scene) return nullptr;

    const double segmentDistSqLimit = ClickArea::segmentDistans * ClickArea::segmentDistans;
    const double vertexRadiusSq = ClickArea::vertexRadius * ClickArea::vertexRadius;

    for (const auto& poly : scene->Polygons) {
        for (size_t i = 0; i < poly->edges.size(); ++i) {
            const auto& edge = poly->edges[i];
            const QPointF v1 = edge->vertices.first->point;
            const QPointF v2 = edge->vertices.second->point;

            if (ignoreNearEndpoints &&
                (distSq(pt, v1) < vertexRadiusSq || distSq(pt, v2) < vertexRadiusSq)) {
                continue;
            }

            if (segmentDistSq(pt, v1, v2) <= segmentDistSqLimit) {
                if (outPoly) *outPoly = poly.get();
                if (outEdgeIdx) *outEdgeIdx = i;
                return edge.get();
            }
        }
    }

    return nullptr;
}