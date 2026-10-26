#include "edgetypes.h"

BezierCurve::BezierCurve(const QPointF& p1, const QPointF& p2) {
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();

    //normal vector
    double nx = dy;
    double ny = -dx;

    double length = std::hypot(nx, ny);
    if (length > 0) {
        nx /= length;
        ny /= length;
    }

    double offset = length * 0.25;

    double base1X = p1.x() + dx / 3.0;
    double base1Y = p1.y() + dy / 3.0;

    double base2X = p1.x() + 2.0 * dx / 3.0;
    double base2Y = p1.y() + 2.0 * dy / 3.0;

    controlP1 = std::make_shared<Vertex>(base1X + nx * offset, base1Y + ny * offset);
    controlP2 = std::make_shared<Vertex>(base2X + nx * offset, base2Y + ny * offset);
}