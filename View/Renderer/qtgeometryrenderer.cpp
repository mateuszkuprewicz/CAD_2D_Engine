#include "QtGeometryRenderer.h"

void QtGeometryRenderer::drawEdge(QPainter& painter, const QPointF& p1, const QPointF& p2, const QPen& pen) {
    painter.setPen(pen);
    painter.drawLine(p1, p2);
}

void QtGeometryRenderer::drawBezier(QPainter& painter, const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPen& pen) {
    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(p0);
    path.cubicTo(p1, p2, p3);
    painter.drawPath(path);
}

void QtGeometryRenderer::drawArc(QPainter& painter, const QPointF& p1, const QPointF& p2, double radius, bool isMajorArc, const QPen& pen) {
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double d = std::hypot(dx, dy);

    if (d < 0.0001) return;

    double R = std::max(std::abs(radius), d / 2.0);
    double h = std::sqrt(R * R - (d / 2.0) * (d / 2.0));
    double nx = -dy / d, ny = dx / d;

    double sign = (radius >= 0) ? 1.0 : -1.0;
    double centerSign = isMajorArc ? -sign : sign;

    QPointF M((p1.x() + p2.x()) / 2.0, (p1.y() + p2.y()) / 2.0);
    QPointF center(M.x() + centerSign * h * nx, M.y() + centerSign * h * ny);

    double startAngle = std::atan2(-(p1.y() - center.y()), p1.x() - center.x()) * 180.0 / M_PI;
    double endAngle = std::atan2(-(p2.y() - center.y()), p2.x() - center.x()) * 180.0 / M_PI;
    double sweepAngle = endAngle - startAngle;

    while (sweepAngle > 180.0) sweepAngle -= 360.0;
    while (sweepAngle <= -180.0) sweepAngle += 360.0;

    if (radius >= 0) {
        if (sweepAngle >= 0.0) sweepAngle -= 360.0;
    } else {
        if (sweepAngle <= 0.0) sweepAngle += 360.0;
    }

    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(p1);
    path.arcTo(center.x() - R, center.y() - R, 2.0 * R, 2.0 * R, startAngle, sweepAngle);
    painter.drawPath(path);
}