#pragma once

class Edge;

class EdgeType{
public:
    virtual void FixEdge() = 0;
    EdgeType(Edge& e) : edge{e}{}
private:
    Edge& edge;
};
