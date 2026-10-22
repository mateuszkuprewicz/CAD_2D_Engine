#include "inputhandler.h"
#include "../Models/polygon.h"
#include "../Models/edge.h"
#include "../Models/vertex.h"
#include "../consts.h"
#include "../Models/Constraints/lengthconstraint.h"
#include "../Models/Constraints/parallelismconstraint.h"
#include "constraintsolver.h"
#include <cmath>
#include <algorithm>
#include <QInputDialog>
#include <QMenu>
#include <QAction>

double InputHandler::distSq(QPointF a, QPointF b) {
    return std::pow(a.x() - b.x(), 2) + std::pow(a.y() - b.y(), 2);
}

double InputHandler::segmentDistSq(QPointF pt, QPointF v1, QPointF v2) {
    double l2 = distSq(v1, v2);
    if (l2 == 0) return distSq(pt, v1);

    double t = ((pt.x() - v1.x()) * (v2.x() - v1.x()) +
                (pt.y() - v1.y()) * (v2.y() - v1.y())) / l2;
    t = std::max(0.0, std::min(1.0, t));

    QPointF proj(v1.x() + t * (v2.x() - v1.x()),
                 v1.y() + t * (v2.y() - v1.y()));

    return std::pow(pt.x() - proj.x(), 2) + std::pow(pt.y() - proj.y(), 2);
}

Edge* InputHandler::findEdgeUnderCursor(QPoint p, const std::vector<Polygon*>& polygons) {
    for (auto poly : polygons) {
        for (const auto& edge : poly->edges) {
            auto v1 = edge->vertices.first;
            auto v2 = edge->vertices.second;
            if (segmentDistSq(p, v1->point, v2->point) <= segmentDistans * segmentDistans) {
                return edge.get();
            }
        }
    }
    return nullptr;
}

bool InputHandler::handleDoubleClick(QMouseEvent *event, const std::vector<Polygon*>& polygons) {
    if (event->button() != Qt::LeftButton) return false;

    QPointF p = event->position();

    for (auto poly : polygons) {
        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto& edge = poly->edges[i];
            QPointF v1 = edge->vertices.first->point;
            QPointF v2 = edge->vertices.second->point;

            if (distSq(p, v1) < vertexRadius * vertexRadius || distSq(p, v2) < vertexRadius * vertexRadius) {
                continue;
            }

            if (segmentDistSq(p, v1, v2) <= segmentDistans * segmentDistans) {
                auto newVertex = std::make_shared<Vertex>(p.x(), p.y());
                auto newEdge1 = std::make_unique<Edge>(edge->vertices.first, newVertex);
                auto newEdge2 = std::make_unique<Edge>(newVertex, edge->vertices.second);

                poly->edges.erase(poly->edges.begin() + i);
                poly->edges.insert(poly->edges.begin() + i, std::move(newEdge2));
                poly->edges.insert(poly->edges.begin() + i, std::move(newEdge1));
                return true;
            }
        }
    }
    return false;
}

bool InputHandler::handleRightClick(QWidget *parent, QMouseEvent *event, const std::vector<Polygon*>& polygons) {
    if (event->button() != Qt::RightButton) return false;

    QPoint p = event->pos();
    const float radiusSq = vertexRadius * vertexRadius;

    for (auto poly : polygons) {
        if (poly->edges.size() <= 3) continue;

        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;
            auto v2 = poly->edges[i]->vertices.second;

            // Vertex deletion logic
            if (distSq(p, v1->point) <= radiusSq) {
                size_t prevIdx = (i == 0) ? poly->edges.size() - 1 : i - 1;

                auto prevVertex = poly->edges[prevIdx]->vertices.first;
                auto nextVertex = poly->edges[i]->vertices.second;

                auto newEdge = std::make_unique<Edge>(prevVertex, nextVertex);

                if (i > 0) {
                    poly->edges.erase(poly->edges.begin() + i);
                    poly->edges.erase(poly->edges.begin() + prevIdx);
                    poly->edges.insert(poly->edges.begin() + prevIdx, std::move(newEdge));
                } else {
                    poly->edges.pop_back();
                    poly->edges.erase(poly->edges.begin());
                    poly->edges.insert(poly->edges.begin(), std::move(newEdge));
                }
                return true;
            }
        }

        // Edge interactions
        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;
            auto v2 = poly->edges[i]->vertices.second;

            if (segmentDistSq(p, v1->point, v2->point) <= segmentDistans * segmentDistans) {

                QMenu menu(parent);
                QAction* lengthAction = menu.addAction("Length constraint");
                QAction* parallelAction = menu.addAction("Paralellism Constraint");

                QAction* selectedAction = menu.exec(event->globalPos());

                if (selectedAction == lengthAction) {
                    bool ok = false;
                    float current_length = poly->edges[i]->GetLength();
                    double new_length = QInputDialog::getDouble(parent, "Edge Length Constraint", "Enter edge length:", current_length, 1.0, 10000.0, 1, &ok);

                    if (ok) {
                        auto constraint = std::make_unique<LengthConstraint>(poly->edges[i].get(), new_length);
                        poly->AddConstraint(std::move(constraint));
                        ConstraintSolver solver;
                        solver.Solve(polygons, nullptr);
                        return true;
                    }
                }
                else if (selectedAction == parallelAction) {
                    // Enter selection mode for the second edge
                    currentMode = AppMode::SelectParallelEdge;
                    firstParallelEdge = poly->edges[i].get();
                    firstParallelPoly = poly;
                    return true;
                }
            }
        }
    }
    return false;
}

bool InputHandler::handleLeftClick(QMouseEvent *event, const std::vector<Polygon*>& polygons) {
    if (event->button() != Qt::LeftButton) return false;

    // Handle parallel constraint edge selection
    if (currentMode == AppMode::SelectParallelEdge) {
        if (hoveredEdge && hoveredEdge != firstParallelEdge) {
            auto constraintOpt = ParallelismConstraint::Create(firstParallelEdge, hoveredEdge);
            if (constraintOpt.has_value()) {
                firstParallelPoly->AddConstraint(std::move(constraintOpt.value()));
                ConstraintSolver solver;
                solver.Solve(polygons, nullptr);
            }
        }

        // Reset state unconditionally after a click in this mode
        currentMode = AppMode::Normal;
        firstParallelEdge = nullptr;
        firstParallelPoly = nullptr;
        hoveredEdge = nullptr;
        return true;
    }

    // Normal click logic (drag elements)
    QPoint p = event->pos();
    const float radiusSq = vertexRadius * vertexRadius;

    for (const auto& poly : polygons) {
        for (const auto& edge : poly->edges) {
            const auto& v = edge->vertices.first;
            if (distSq(p, v->point) <= radiusSq) {
                lastMousePos = p;
                draggedVertex = v.get();
                return true;
            }
        }
    }

    for (auto poly : polygons) {
        if (poly->isClicked(p)) {
            lastMousePos = p;
            draggedPolygon = poly;
            return true;
        }
    }

    return false;
}

bool InputHandler::handleMouseMove(QMouseEvent *event, const std::vector<Polygon*>& polygons) {
    bool requireRedraw = false;
    QPoint currentPos = event->pos();

    // Handle edge highlighting when selecting a parallel pair
    if (currentMode == AppMode::SelectParallelEdge) {
        Edge* foundEdge = findEdgeUnderCursor(currentPos, polygons);
        if (foundEdge != hoveredEdge) {
            hoveredEdge = foundEdge;
            requireRedraw = true;
        }
    }

    // Handle dragging logic
    if (draggedVertex || draggedPolygon) {
        int dx = currentPos.x() - lastMousePos.x();
        int dy = currentPos.y() - lastMousePos.y();

        if (draggedVertex) {
            draggedVertex->point.rx() += dx;
            draggedVertex->point.ry() += dy;
            ConstraintSolver solver;
            solver.Solve(polygons, draggedVertex);
        } else if (draggedPolygon) {
            draggedPolygon->translate(dx, dy);
        }

        lastMousePos = currentPos;
        requireRedraw = true;
    }

    return requireRedraw;
}

bool InputHandler::handleMouseRelease(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (draggedPolygon || draggedVertex) {
            draggedPolygon = nullptr;
            draggedVertex = nullptr;
            return true; // Optional: redraw if UI state visual changes upon release
        }
    }
    return false;
}