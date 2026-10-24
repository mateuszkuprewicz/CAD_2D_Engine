#pragma once
#include "edge.h"
#include <vector>
#include <unordered_set>
#include <memory>
#include <QPoint>
#include "Constraints/abstractconstraint.h"

class Polygon
{
public:
    Polygon(std::initializer_list<QPoint>);
    std::vector<std::unique_ptr<Edge>> edges;

    void translate(int dx, int dy);
    bool isClicked(QPoint clickPos) const;
    void addVertex(std::shared_ptr<Vertex> newVertex, int edgeIdx);
    void delVertex(Vertex* vertex, int prevEdgeIdx, int nextEdgeIdx);

private:
    std::unordered_set<Vertex *> vertices;
};

