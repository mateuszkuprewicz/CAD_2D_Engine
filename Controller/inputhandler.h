#pragma once
#include <QMouseEvent>
#include <QPoint>
#include <vector>

class Polygon;
class Vertex;

class InputHandler
{
public:
    static void handleDoubleClick(QMouseEvent *event, const std::vector<Polygon*> polygons);
    static bool handleRightClick(QWidget *, QMouseEvent *event, const std::vector<Polygon*> polygons);
    static std::pair<Polygon*, Vertex*> handleLeftClick(QMouseEvent *event, const std::vector<Polygon*> polygons, QPoint& outLastPos);
    static bool handleMouseMove(QMouseEvent *event, const std::vector<Polygon*> polygons, Polygon* draggedPolygon, Vertex* draggedVertex, QPoint& lastPos);
    static void handleMouseRelease(QMouseEvent *event, Polygon*& draggedPolygon, Vertex*& draggedVertex);

private:
    static double distSq(QPointF a, QPointF b);
    static double segmentDistSq(QPointF pt, QPointF v1, QPointF v2);
};