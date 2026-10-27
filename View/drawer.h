#pragma once
#include <QPainter>
#include <memory>
#include "Renderer/IGeometryRenderer.h"

// Forward declarations
class Scene;
class Edge;

class Drawer {
public:
    explicit Drawer(Scene *s);
    void Render(QPainter &painter);

    void setRenderer(std::unique_ptr<IGeometryRenderer> r) { renderer = std::move(r); }
    IGeometryRenderer* getRenderer() const { return renderer.get(); }
    void SetHoveredEdge(const Edge *);

private:
    void DrawPolygons(QPainter &painter);
    void DrawConstraints(QPainter &painter);

    Scene *scene = nullptr;
    const Edge *hoveredEdge = nullptr;
    std::unique_ptr<IGeometryRenderer> renderer;
};