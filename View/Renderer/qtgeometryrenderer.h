#pragma once
#include "IGeometryRenderer.h"
#include <QPainterPath>
#include <cmath>

class QtGeometryRenderer : public IGeometryRenderer {
public:
    void drawEdge(QPainter& painter, const QPointF& p1, const QPointF& p2, const QPen& pen) override;
    void drawBezier(QPainter& painter, const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPen& pen) override;
    void drawArc(QPainter& painter, const QPointF& p1, const QPointF& p2, double radius, bool isMajorArc, const QPen& pen) override;
};