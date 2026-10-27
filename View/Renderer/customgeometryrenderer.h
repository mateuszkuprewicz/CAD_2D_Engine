#pragma once
#include "IGeometryRenderer.h"
#include <cmath>

class CustomGeometryRenderer : public IGeometryRenderer {
private:
    void drawBresenhamLine(QPainter& painter, int x1, int y1, int x2, int y2, const QPen& pen);
    void drawMidpointArc(QPainter& painter, int cx, int cy, int R, double startAngle, double sweepAngle, const QPen& pen);
    void plotArcPoints(QPainter& painter, int cx, int cy, int x, int y, double startAngle, double sweepAngle, const QPen& pen);
    double normalizeAngle(double angle);

public:
    void drawEdge(QPainter& painter, const QPointF& p1, const QPointF& p2, const QPen& pen) override;
    void drawBezier(QPainter& painter, const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPen& pen) override;
    void drawArc(QPainter& painter, const QPointF& p1, const QPointF& p2, double radius, bool isMajorArc, const QPen& pen) override;
};