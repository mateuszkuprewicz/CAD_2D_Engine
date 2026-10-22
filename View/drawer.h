#pragma once
#include <vector>
#include <QPainter>

class Polygon;
class Edge;
class QImage;

class Drawer
{
public:
    Drawer(std::vector<Polygon *>);
    void Render(QPainter& painter);
    void SetHoveredEdge(const Edge* edge) { hoveredEdge = edge; }
private:
    std::vector<const Polygon*> polygons;
    const Edge* hoveredEdge = nullptr;
};


