#pragma once
#include <QMouseEvent>
#include <QPoint>
#include <vector>

class Polygon;
class Vertex;
class Edge;

enum class AppMode { Normal, SelectParallelEdge };

class InputHandler
{
public:
    InputHandler() = default;

    bool handleDoubleClick(QMouseEvent *event, const std::vector<Polygon*>& polygons);
    bool handleRightClick(QWidget* parent, QMouseEvent *event, const std::vector<Polygon*>& polygons);
    bool handleLeftClick(QMouseEvent *event, const std::vector<Polygon*>& polygons);
    bool handleMouseMove(QMouseEvent *event, const std::vector<Polygon*>& polygons);
    bool handleMouseRelease(QMouseEvent *event);

    const Edge* getHoveredEdge() const { return hoveredEdge; }

private:
    double distSq(QPointF a, QPointF b);
    double segmentDistSq(QPointF pt, QPointF v1, QPointF v2);
    Edge* findEdgeUnderCursor(QPoint p, const std::vector<Polygon*>& polygons);

    AppMode currentMode = AppMode::Normal;
    Edge* firstParallelEdge = nullptr;
    Polygon* firstParallelPoly = nullptr;
    Edge* hoveredEdge = nullptr;

    Polygon* draggedPolygon = nullptr;
    Vertex* draggedVertex = nullptr;
    QPoint lastMousePos;
};