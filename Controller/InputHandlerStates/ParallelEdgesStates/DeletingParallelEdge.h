#pragma once
#include "AbstractParallelEdgeState.h"

class DeletingParallelEdge : public AbstractParallelEdgeState
{
public:
    using AbstractParallelEdgeState::AbstractParallelEdgeState;

protected:
    void applyParallelAction(Edge* targetEdge) override;
};