#include "vertex.h"

Vertex::Vertex(double x, double y) {
    point = QPointF(x,y);
}

Vertex::Vertex(const Vertex& other) {
    point = other.point;
}

Vertex& Vertex::operator=(const Vertex& other) {
    point = other.point;
    return *this;
}
