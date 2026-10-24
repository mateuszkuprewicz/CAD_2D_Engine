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
    vertices.insert(first.get());
    vertices.insert(temp.get());

    std::shared_ptr<Vertex> second;
    for(++it; it!=list.end(); ++it)
    {
        second = std::make_shared<Vertex>(it->x(), it->y());
        edges.push_back(std::make_unique<Edge>(temp,second));
        vertices.insert(second.get());
        temp = second;
    }
    edges.push_back(std::make_unique<Edge>(second, first));
}

Polygon::Polygon(const Polygon& p) {
    const auto &list = p.edges;
    auto it = list.begin();
    if (list.size() < 3) return;

    std::shared_ptr<Vertex> first = std::make_shared<Vertex>((*it)->vertices.first->point.x(), (*it)->vertices.first->point.y());
    ++it;
    std::shared_ptr<Vertex> temp = std::make_shared<Vertex>((*it)->vertices.second->point.x(), (*it)->vertices.second->point.y());
    std::unique_ptr temp_edge = std::make_unique<Edge>(first,temp);
    edges.push_back(std::move(temp_edge));
    vertices.insert(first.get());
    vertices.insert(temp.get());

    std::shared_ptr<Vertex> second;
    for(++it; it!=list.end(); ++it)
    {
        second = std::make_shared<Vertex>((*it)->vertices.second->point.x(), (*it)->vertices.second->point.y());
        edges.push_back(std::make_unique<Edge>(temp,second));
        vertices.insert(second.get());
        temp = second;
    }
    edges.push_back(std::make_unique<Edge>(second, first));
}

Polygon& Polygon::operator=(Polygon temp){
    std::swap(this->edges, temp.edges);
    std::swap(this->vertices, temp.vertices);
    return *this;
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

void Polygon::addVertex(std::shared_ptr<Vertex> newVertex, int edgeIdx) {
    auto newEdge1 = std::make_unique<Edge>(edges[edgeIdx]->vertices.first, newVertex);
    auto newEdge2 = std::make_unique<Edge>(newVertex, edges[edgeIdx]->vertices.second);

    edges.erase(edges.begin() + edgeIdx);
    edges.insert(edges.begin() + edgeIdx, std::move(newEdge2));
    edges.insert(edges.begin() + edgeIdx, std::move(newEdge1));

    vertices.insert(newVertex.get());
}

void Polygon::delVertex(Vertex* vertex, int prevEdgeIdx, int nextEdgeIdx) {
    std::shared_ptr<Vertex> prevVertex = edges[prevEdgeIdx]->vertices.first;
    std::shared_ptr<Vertex> nextVertex = edges[nextEdgeIdx]->vertices.second;

    if(nextEdgeIdx != 0) {
        edges.erase(edges.begin() + nextEdgeIdx);
        edges.erase(edges.begin() + prevEdgeIdx);
        edges.insert(edges.begin() + prevEdgeIdx, std::make_unique<Edge>(prevVertex, nextVertex));
    }
    else {
        edges.erase(edges.begin() + prevEdgeIdx);
        edges.erase(edges.begin());
        edges.insert(edges.begin(), std::make_unique<Edge>(prevVertex, nextVertex));
    }

    vertices.erase(vertex);
}