#pragma once
#include <QMouseEvent>
#include <QPoint>

class Polygon;
class Vertex;
class Edge;
class Scene;
class QWidget;

enum class AppMode { Normal, SelectParallelEdge };

class InputHandler
{
public:
    explicit InputHandler(Scene* scene);

    bool handleDoubleClick(QMouseEvent *event);
    bool handleRightClick(QWidget* parent, QMouseEvent *event);
    bool handleLeftClick(QMouseEvent *event);
    bool handleMouseMove(QMouseEvent *event);
    bool handleMouseRelease(QMouseEvent *event);

    const Edge* getHoveredEdge() const { return hoveredEdge; }

private:
    double distSq(QPointF a, QPointF b);
    double segmentDistSq(QPointF pt, QPointF v1, QPointF v2);
    Edge* findEdgeUnderCursor(QPoint p);

    Scene* scene = nullptr;
    AppMode currentMode = AppMode::Normal;
    Edge* firstParallelEdge = nullptr;
    Polygon* firstParallelPoly = nullptr;
    Edge* hoveredEdge = nullptr;

    Polygon* draggedPolygon = nullptr;
    Vertex* draggedVertex = nullptr;
    QPoint lastMousePos;
};