#pragma once
#include <QPoint>

class Edge;
class AbstractConstraint;

class Vertex
{
public:
    Vertex(double, double);
    QPointF point;
};
