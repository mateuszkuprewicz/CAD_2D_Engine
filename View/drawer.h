#pragma once
#include <vector>

class Polygon;
class Edge;
class QImage;

class Drawer
{
public:
    Drawer(QImage&, std::initializer_list<Polygon *>);
    void Render();
private:
    std::vector<Polygon *> polygons;
    void drawEdge(const Edge&);
    QImage &image;
};


