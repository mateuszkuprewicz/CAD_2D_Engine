#include "CustomGeometryRenderer.h"

// Generalized Bresenham algorithm
void CustomGeometryRenderer::drawBresenhamLine(QPainter& painter, int x1, int y1, int x2, int y2, const QPen& pen) {
    painter.setPen(pen.color());

    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    bool isDashed = (pen.style() == Qt::DashLine);
    int dashLength = 6;
    int gapLength = 6;
    int step = 0;

    while (true) {
        bool drawPoint = true;
        if (isDashed) {
            if ((step % (dashLength + gapLength)) >= dashLength) {
                drawPoint = false;
            }
        }

        if (drawPoint) {
            painter.drawPoint(x1, y1);
        }
        step++;

        if (x1 == x2 && y1 == y2) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy) { err += dx; y1 += sy; }
    }
}

void CustomGeometryRenderer::drawEdge(QPainter& painter, const QPointF& p1, const QPointF& p2, const QPen& pen) {
    drawBresenhamLine(painter, std::round(p1.x()), std::round(p1.y()), std::round(p2.x()), std::round(p2.y()), pen);
}

// Power basis algorithm for Bezier curve
void CustomGeometryRenderer::drawBezier(QPainter& painter, const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPen& pen) {
    // Coefficients for power basis: P(t) = a*t^3 + b*t^2 + c*t + d
    QPointF d = p0;
    QPointF c = 3.0 * (p1 - p0);
    QPointF b = 3.0 * (p2 - 2.0 * p1 + p0);
    QPointF a = p3 - 3.0 * p2 + 3.0 * p1 - p0;

    int segments = 100;
    QPointF prevPoint = p0;

    for (int i = 1; i <= segments; ++i) {
        double t = static_cast<double>(i) / segments;
        QPointF nextPoint = ((a * t + b) * t + c) * t + d;
        drawEdge(painter, prevPoint, nextPoint, pen);
        prevPoint = nextPoint;
    }
}

double CustomGeometryRenderer::normalizeAngle(double angle) {
    while (angle < 0.0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

void CustomGeometryRenderer::plotArcPoints(QPainter& painter, int cx, int cy, int x, int y, double startAngle, double sweepAngle, const QPen& pen) {
    QPoint pts[8] = {
        {cx + x, cy + y}, {cx - x, cy + y},
        {cx + x, cy - y}, {cx - x, cy - y},
        {cx + y, cy + x}, {cx - y, cy + x},
        {cx + y, cy - x}, {cx - y, cy - x}
    };

    painter.setPen(pen.color());

    for (const auto& p : pts) {
        double angle = std::atan2(-(p.y() - cy), p.x() - cx) * 180.0 / M_PI;
        angle = normalizeAngle(angle);

        double nStart = normalizeAngle(startAngle);
        double nEnd = nStart + sweepAngle;

        bool inRange = false;
        if (sweepAngle >= 0) {
            if (nEnd >= 360.0) {
                inRange = (angle >= nStart || angle <= nEnd - 360.0);
            } else {
                inRange = (angle >= nStart && angle <= nEnd);
            }
        } else {
            if (nEnd < 0) {
                inRange = (angle <= nStart || angle >= nEnd + 360.0);
            } else {
                inRange = (angle <= nStart && angle >= nEnd);
            }
        }

        if (inRange) {
            painter.drawPoint(p);
        }
    }
}

// Midpoint circle algorithm restricted to arc angles
void CustomGeometryRenderer::drawMidpointArc(QPainter& painter, int cx, int cy, int R, double startAngle, double sweepAngle, const QPen& pen) {
    int d = 1 - R;
    int x = 0;
    int y = R;

    plotArcPoints(painter, cx, cy, x, y, startAngle, sweepAngle, pen);

    while (y > x) {
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * x - 2 * y + 5;
            y--;
        }
        x++;
        plotArcPoints(painter, cx, cy, x, y, startAngle, sweepAngle, pen);
    }
}

void CustomGeometryRenderer::drawArc(QPainter& painter, const QPointF& p1, const QPointF& p2, double radius, bool isMajorArc, const QPen& pen) {
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double dist = std::hypot(dx, dy);

    if (dist < 0.0001) return;

    double R = std::max(std::abs(radius), dist / 2.0);
    double h = std::sqrt(R * R - (dist / 2.0) * (dist / 2.0));
    double nx = -dy / dist, ny = dx / dist;

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

    drawMidpointArc(painter, std::round(center.x()), std::round(center.y()), std::round(R), startAngle, sweepAngle, pen);
}