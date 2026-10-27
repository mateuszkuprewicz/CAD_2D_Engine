#pragma once
#include <memory>
#include <cmath>
#include "vertex.h"


class BezierCurve {
public:
    BezierCurve(const QPointF& p1, const QPointF& p2);
    std::unique_ptr<Vertex> controlP1;
    std::unique_ptr<Vertex> controlP2;
};

class ArcCurve {
public:
    ArcCurve(QPointF& p1, const QPointF& p2);
    double radius;
    bool isMajorArc = false;
};