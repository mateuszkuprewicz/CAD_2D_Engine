#pragma once
#include <vector>
#include <memory>

class ConstraintManager;
class Vertex; class Edge; class Polygon;
class AbstractConstraint;
class Scene;

struct ContinuityState {
    bool isG1 = false;
    bool isC1 = false;
};

class ContinuityConstraintManager {
public:
    static std::vector<std::shared_ptr<AbstractConstraint>> HandleEdgeChange(ConstraintManager* cm, Vertex* sharedVertex, Edge* unchangedEdge);
    static ContinuityState SaveState(ConstraintManager* cm, Vertex* v);
    static void RestoreState(Scene* scene, Polygon* poly, Vertex* v, ContinuityState state);
};