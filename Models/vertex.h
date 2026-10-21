#pragma once
#include <QPoint>
#include <utility>

class Edge;

class Vertex
{
public:
    Vertex(double, double);
    QPointF point;
private:
    std::pair<Edge*, Edge*> edges;
};
