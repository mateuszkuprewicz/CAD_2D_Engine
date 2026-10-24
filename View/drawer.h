#pragma once
#include <QPainter>
#include "../Models/scene.h"

class Polygon;
class Edge;
class QImage;

class Drawer
{
public:
    Drawer(Scene* s);
    void Render(QPainter& painter);
    void SetHoveredEdge(const Edge* edge) { hoveredEdge = edge; }
private:
    const Edge* hoveredEdge = nullptr;
    Scene *scene;
};


