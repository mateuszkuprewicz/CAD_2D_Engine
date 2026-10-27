#include "edge.h"
#include "vertex.h"
#include <cmath>


Edge::Edge(std::shared_ptr<Vertex> v1, std::shared_ptr<Vertex>v2) {
    vertices.first = v1;
    vertices.second = v2;
    bezier = nullptr;
    arc = nullptr;
}

float Edge::GetLength() const {
    return std::hypot(vertices.first->point.x() - vertices.second->point.x(), vertices.first->point.y() - vertices.second->point.y());
}

Edge& Edge::operator=(const Edge& other) {
    vertices.first->point = other.vertices.first->point;
    vertices.second->point = other.vertices.second->point;
    if(other.isBezier()) {
        if(bezier == nullptr)
            bezier = std::make_unique<BezierCurve>(other.vertices.first->point, other.vertices.second->point);
        bezier->controlP1->point = other.bezier->controlP1->point;
        bezier->controlP2->point = other.bezier->controlP2->point;
        arc = nullptr;
    }
    else if(other.isArc()) {
        if(arc == nullptr)
            arc = std::make_unique<ArcCurve>(other.vertices.first->point, other.vertices.second->point);
        arc->radius = other.arc->radius;
        arc->isMajorArc = other.arc->isMajorArc;
        bezier = nullptr;
    }
    else {
        arc = nullptr;
        bezier = nullptr;
    }
    return *this;
}

Edge::Edge(const Edge& other) {
    vertices.first = std::make_shared<Vertex>(*other.vertices.first);
    vertices.second = std::make_shared<Vertex>(*other.vertices.second);
    if(other.isBezier()) {
        bezier = std::make_unique<BezierCurve>(other.vertices.first->point, other.vertices.second->point);
        bezier->controlP1 = std::make_unique<Vertex>(*other.bezier->controlP1);
        bezier->controlP2 = std::make_unique<Vertex>(*other.bezier->controlP2);
        arc = nullptr;
    }
    else if(other.isArc()) {
        arc = std::make_unique<ArcCurve>(other.vertices.first->point, other.vertices.second->point);
        arc->radius = other.arc->radius;
        arc->isMajorArc = other.arc->isMajorArc;
        bezier = nullptr;
    }
    else {
        arc = nullptr;
        bezier = nullptr;
    }
}