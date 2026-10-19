#pragma once
#include <QPoint>
#include <utility>

class Edge;

class Vertex
{
public:
    Vertex(int, int);
    QPoint point;
private:
    std::pair<Edge*, Edge*> edges;
};
