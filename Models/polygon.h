#pragma once
#include "edge.h"
#include <vector>
#include <memory>
#include <QPoint>

class Polygon
{
public:
    Polygon(std::initializer_list<QPoint>);
    std::vector<std::unique_ptr<Edge>> edges;
    void translate(int dx, int dy);
    bool isClicked(QPoint clickPos) const;
};

