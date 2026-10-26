#pragma once
#include <memory>
#include <cmath>
#include "vertex.h"


class BezierCurve {
public:
    BezierCurve(const QPointF& p1, const QPointF& p2);
    std::shared_ptr<Vertex> controlP1;
    std::shared_ptr<Vertex> controlP2;
};