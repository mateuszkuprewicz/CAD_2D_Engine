#pragma once
#include "../AbstractInputHandlerState.h"

class Edge;
class Polygon;

class AbstractParallelEdgeState : public AbstractInputHandlerState
{
public:
    AbstractParallelEdgeState(InputHandler* context,
                              Scene* scene,
                              Edge* firstParallelEdge,
                              Polygon* firstParallelPoly);

    bool handleLeftClick(QMouseEvent* event) override;
    bool handleMouseMove(QMouseEvent* event) override;

    const Edge* getHoveredEdge() const override { return hoveredEdge; }

protected:
    virtual void applyParallelAction(Edge* targetEdge) = 0;

    Edge* firstParallelEdge = nullptr;
    Polygon* firstParallelPoly = nullptr;
    Edge* hoveredEdge = nullptr;
};