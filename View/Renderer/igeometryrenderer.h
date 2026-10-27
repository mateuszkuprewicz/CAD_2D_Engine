#pragma once
#include <QPainter>
#include <QPointF>
#include <QPen>

class IGeometryRenderer {
public:
    virtual ~IGeometryRenderer() = default;

    virtual void drawEdge(QPainter& painter, const QPointF& p1, const QPointF& p2, const QPen& pen) = 0;
    virtual void drawBezier(QPainter& painter, const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPen& pen) = 0;
    virtual void drawArc(QPainter& painter, const QPointF& p1, const QPointF& p2, double radius, bool isMajorArc, const QPen& pen) = 0;
};