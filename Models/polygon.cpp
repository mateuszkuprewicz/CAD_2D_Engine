#include "polygon.h"
#include "vertex.h"

Polygon::Polygon(std::initializer_list<QPoint> list) {
    auto it = list.begin();
    if (list.size() < 3) return;

    std::shared_ptr<Vertex> first = std::make_shared<Vertex>(it->x(), it->y());
    ++it;
    std::shared_ptr<Vertex> temp = std::make_shared<Vertex>(it->x(), it->y());
    std::unique_ptr temp_edge = std::make_unique<Edge>(first,temp);
    edges.push_back(std::move(temp_edge));

    std::shared_ptr<Vertex> second;
    for(++it; it!=list.end(); ++it)
    {
        second = std::make_shared<Vertex>(it->x(), it->y());
        edges.push_back(std::make_unique<Edge>(temp,second));
        temp = second;
    }
    edges.push_back(std::make_unique<Edge>(second, first));
}

void Polygon::translate(int dx, int dy) {
    for(auto it = edges.begin(); it != edges.end(); ++it) {
        auto edge = (*it)->vertices;
        edge.first->point.setX(edge.first->point.x() + dx);
        edge.first->point.setY(edge.first->point.y() + dy);
    }
}

bool Polygon::isClicked(QPoint clickPos) const {
    if (edges.empty()) return false;

    //borders of a rectangle encompassing the polygon
    int minX = edges[0]->vertices.first->point.x();
    int maxX = minX;
    int minY = edges[0]->vertices.first->point.y();
    int maxY = minY;

    for (const auto& edge : edges) {
        int x = edge->vertices.first->point.x();
        int y = edge->vertices.first->point.y();
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }

    return (clickPos.x() >= minX && clickPos.x() <= maxX &&
            clickPos.y() >= minY && clickPos.y() <= maxY);
}

void Polygon::AddConstraint(std::unique_ptr<AbstractConstraint> c) {
    constraints.push_back(std::move(c));
}

std::vector<const AbstractConstraint*> Polygon::Constraints() const {
    std::vector<const AbstractConstraint*> result;
    result.reserve(constraints.size());
    for (const auto& ptr : constraints) {
        result.push_back(ptr.get());
    }
    return result;
}