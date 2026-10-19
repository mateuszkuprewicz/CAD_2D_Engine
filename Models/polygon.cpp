#include "polygon.h"
#include "vertex.h"

Polygon::Polygon(std::initializer_list<QPoint> list) {
    auto it = list.begin();
    std::shared_ptr<Vertex> first = std::make_shared<Vertex>(Vertex(it->x(), it->y()));
    ++it;
    std::shared_ptr<Vertex> temp = std::make_shared<Vertex>(Vertex(it->x(), it->y()));
    std::unique_ptr temp_edge = std::make_unique<Edge>(Edge(first,temp));
    edges.push_back(std::move(temp_edge));

    std::shared_ptr<Vertex> second;
    for(; it!=list.end(); ++it)
    {
        second = std::make_shared<Vertex>(Vertex(it->x(), it->y()));
        edges.push_back(std::make_unique<Edge>(Edge(temp,second)));
        temp = second;
    }
    edges.push_back(std::make_unique<Edge>(Edge(second, first)));
}
