#pragma once
#include "AbstractParallelEdgeState.h"

class SelectingParallelEdge : public AbstractParallelEdgeState
{
public:
    using AbstractParallelEdgeState::AbstractParallelEdgeState;

protected:
    void applyParallelAction(Edge* targetEdge) override;
};