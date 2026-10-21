#include "inputhandler.h"
#include "../Models/polygon.h"
#include "../Models/edge.h"
#include "../Models/vertex.h"
#include "../consts.h"
#include <cmath>
#include <algorithm>
#include <QInputDialog>
#include "../Models/Constraints/lengthconstraint.h"
#include "constraintsolver.h"

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

void InputHandler::handleDoubleClick(QMouseEvent *event, const std::vector<Polygon*> polygons) {
    if (event->button() != Qt::LeftButton) return;

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
                return;
            }
        }
    }
}

bool InputHandler::handleRightClick(QWidget *parent, QMouseEvent *event, const std::vector<Polygon*> polygons) {
    if (event->button() != Qt::RightButton) return false;

    QPoint p = event->pos();
    const float radiusSq = vertexRadius * vertexRadius;

    for (auto poly : polygons) {
        if (poly->edges.size() <= 3) continue;

        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;
            auto v2 = poly->edges[i]->vertices.second;

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
        //adding a length constraint
        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;
            auto v2 = poly->edges[i]->vertices.second;
            if(segmentDistSq(p, v1->point, v2->point) <= segmentDistans * segmentDistans) {
                bool ok = false;
                float current_length = poly->edges[i]->getLength();
                double new_length = QInputDialog::getDouble(
                    parent,
                    "Edge Length Constraint",
                    "Enter edge length:",
                    current_length,
                    1.0,
                    10000.0,
                    1,
                    &ok
                    );

                if (ok) {
                    auto constraint = std::make_unique<LengthConstraint>(poly->edges[i].get(), new_length);
                    poly->AddConstraint(std::move(constraint));
                    ConstraintSolver solver;
                    solver.Solve(polygons, nullptr);
                    return true;
                }
            }

        }



    }
    return false;
}

std::pair<Polygon*, Vertex*> InputHandler::handleLeftClick(QMouseEvent *event, const std::vector<Polygon*> polygons, QPoint& outLastPos) {
    if (event->button() != Qt::LeftButton) return {nullptr, nullptr};

    QPoint p = event->pos();
    const float radiusSq = vertexRadius * vertexRadius;

    for (const auto& poly : polygons) {
        for (const auto& edge : poly->edges) {
            const auto& v = edge->vertices.first;
            if (distSq(p, v->point) <= radiusSq) {
                outLastPos = p;
                return {nullptr, v.get()};
            }
        }
    }

    for (auto poly : polygons) {
        if (poly->isClicked(p)) {
            outLastPos = p;
            return {poly, nullptr};
        }
    }

    return {nullptr, nullptr};
}

bool InputHandler::handleMouseMove(QMouseEvent *event, const std::vector<Polygon*> polygons, Polygon* draggedPolygon, Vertex* draggedVertex, QPoint& lastPos) {
    if (!draggedPolygon && !draggedVertex) return false;

    QPoint currentPos = event->pos();
    int dx = currentPos.x() - lastPos.x();
    int dy = currentPos.y() - lastPos.y();

    if (draggedVertex) {
        draggedVertex->point.rx() += dx;
        draggedVertex->point.ry() += dy;
        ConstraintSolver solver;
        solver.Solve(polygons, draggedVertex);
    } else if (draggedPolygon) {
        draggedPolygon->translate(dx, dy);
    }

    lastPos = currentPos;
    return true;
}

void InputHandler::handleMouseRelease(QMouseEvent *event, Polygon*& draggedPolygon, Vertex*& draggedVertex) {
    if (event->button() == Qt::LeftButton) {
        draggedPolygon = nullptr;
        draggedVertex = nullptr;
    }
}
