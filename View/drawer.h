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
private:
    std::vector<const Polygon*> polygons;
};


