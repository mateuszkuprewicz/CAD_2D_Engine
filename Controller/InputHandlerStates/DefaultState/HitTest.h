#pragma once
#include <QPointF>
#include <cstddef>

class Scene;
class Polygon;
class Vertex;
class Edge;

class HitTest
{
public:
    Vertex* VertexHitTest(Scene* scene,
                          QPointF pt,
                          bool includeControlPoints = false,
                          Polygon** outPoly = nullptr,
                          size_t* outEdgeIdx = nullptr) const;

    Edge* EdgeHitTest(Scene* scene,
                      QPointF pt,
                      bool ignoreNearEndpoints = false,
                      Polygon** outPoly = nullptr,
                      size_t* outEdgeIdx = nullptr) const;

private:
    double distSq(QPointF a, QPointF b) const;
    double segmentDistSq(QPointF pt, QPointF v1, QPointF v2) const;
};