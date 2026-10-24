#include "inputhandler.h"
#include "../Models/scene.h"
#include "../Models/polygon.h"
#include "../Models/edge.h"
#include "../Models/vertex.h"
#include "../consts.h"
#include "../Models/Constraints/lengthconstraint.h"
#include "../Models/Constraints/parallelismconstraint.h"
#include <cmath>
#include <algorithm>
#include <QInputDialog>
#include <QMenu>
#include <QAction>

InputHandler::InputHandler(Scene* scene) : scene(scene) {}

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

Edge* InputHandler::findEdgeUnderCursor(QPoint p) {
    if (!scene) return nullptr;

    for (const auto& poly : scene->Polygons) {
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

bool InputHandler::handleDoubleClick(QMouseEvent *event) {
    if (!scene || event->button() != Qt::LeftButton) return false;

    QPointF p = event->position();

    for (const auto& poly : scene->Polygons) {
        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto& edge = poly->edges[i];
            QPointF v1 = edge->vertices.first->point;
            QPointF v2 = edge->vertices.second->point;

            if (distSq(p, v1) < vertexRadius * vertexRadius || distSq(p, v2) < vertexRadius * vertexRadius) {
                continue;
            }

            if (segmentDistSq(p, v1, v2) <= segmentDistans * segmentDistans) {
                auto newVertex = std::make_shared<Vertex>(p.x(), p.y());
                scene->addVertex(poly.get(), std::move(newVertex), i);
                return true;
            }
        }
    }
    return false;
}

bool InputHandler::handleRightClick(QWidget *parent, QMouseEvent *event) {
    if (!scene || event->button() != Qt::RightButton) return false;

    QPoint p = event->pos();
    const float radiusSq = vertexRadius * vertexRadius;

    for (const auto& poly : scene->Polygons) {
        if (poly->edges.size() <= 3) continue;

        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;

            if (distSq(p, v1->point) <= radiusSq) {
                size_t prevIdx = (i == 0) ? poly->edges.size() - 1 : i - 1;
                scene->delVertex(poly.get(), v1.get(), prevIdx, i);
                return true;
            }
        }

        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;
            auto v2 = poly->edges[i]->vertices.second;

            if (segmentDistSq(p, v1->point, v2->point) <= segmentDistans * segmentDistans) {

                bool hasLength = scene->constraintManager.HasEdgeConstraint<LengthConstraint>(v1.get(), v2.get());
                bool hasParallel = scene->constraintManager.HasEdgeConstraint<ParallelismConstraint>(v1.get(), v2.get());

                QMenu menu(parent);

                QMenu* lengthMenu = menu.addMenu("Length Constraint");
                QAction* addLengthAction = lengthMenu->addAction("Add / Edit");
                QAction* delLengthAction = lengthMenu->addAction("Delete");
                delLengthAction->setEnabled(hasLength);

                QMenu* parallelMenu = menu.addMenu("Parallelism Constraint");
                QAction* addParallelAction = parallelMenu->addAction("Add");
                QAction* delParallelAction = parallelMenu->addAction("Delete");
                delParallelAction->setEnabled(hasParallel);

                QAction* selectedAction = menu.exec(event->globalPos());

                if (selectedAction == addLengthAction) {
                    bool ok = false;
                    float current_length = poly->edges[i]->GetLength();
                    double new_length = QInputDialog::getDouble(parent, "Edge Length Constraint", "Enter edge length:", current_length, 1.0, 10000.0, 1, &ok);

                    if (ok) {
                        auto constraint = std::make_shared<LengthConstraint>(poly->edges[i].get(), new_length);
                        scene->constraintManager.AddConstraint(constraint);
                        scene->constraintManager.SolveConstraints(nullptr);
                        return true;
                    }
                }
                else if (selectedAction == delLengthAction) {
                    auto* c = scene->constraintManager.GetLengthConstraint(v1.get(), v2.get());
                    if (c) {
                        scene->constraintManager.DelConstraint(c);
                        scene->constraintManager.SolveConstraints(nullptr);
                    }
                    return true;
                }
                else if (selectedAction == addParallelAction) {
                    currentMode = AppMode::SelectParallelEdge;
                    firstParallelEdge = poly->edges[i].get();
                    firstParallelPoly = poly.get();
                    return true;
                }
                else if (selectedAction == delParallelAction) {
                    currentMode = AppMode::DeleteParallelEdge;
                    firstParallelEdge = poly->edges[i].get();
                    firstParallelPoly = poly.get();
                    return true;
                }
            }
        }
    }
    return false;
}

bool InputHandler::handleLeftClick(QMouseEvent *event) {
    if (!scene || event->button() != Qt::LeftButton) return false;

    if (currentMode == AppMode::SelectParallelEdge || currentMode == AppMode::DeleteParallelEdge) {
        if (hoveredEdge && hoveredEdge != firstParallelEdge) {
            if (currentMode == AppMode::SelectParallelEdge) {
                auto constraintOpt = ParallelismConstraint::Create(firstParallelEdge, hoveredEdge);
                if (constraintOpt.has_value()) {
                    scene->constraintManager.AddConstraint(std::move(constraintOpt.value()));
                    scene->constraintManager.SolveConstraints(nullptr);
                }
            } else {
                auto* c = scene->constraintManager.GetParallelConstraint(firstParallelEdge, hoveredEdge);
                if (c) {
                    scene->constraintManager.DelConstraint(c);
                    scene->constraintManager.SolveConstraints(nullptr);
                }
            }
        }

        currentMode = AppMode::Normal;
        firstParallelEdge = nullptr;
        firstParallelPoly = nullptr;
        hoveredEdge = nullptr;
        return true;
    }

    QPoint p = event->pos();
    const float radiusSq = vertexRadius * vertexRadius;

    for (const auto& poly : scene->Polygons) {
        for (const auto& edge : poly->edges) {
            const auto& v = edge->vertices.first;
            if (distSq(p, v->point) <= radiusSq) {
                lastMousePos = p;
                draggedVertex = v.get();
                return true;
            }
        }
    }

    for (const auto& poly : scene->Polygons) {
        if (poly->isClicked(p)) {
            lastMousePos = p;
            draggedPolygon = poly.get();
            return true;
        }
    }

    return false;
}

bool InputHandler::handleMouseMove(QMouseEvent *event) {
    if (!scene) return false;

    bool requireRedraw = false;
    QPoint currentPos = event->pos();

    if (currentMode == AppMode::SelectParallelEdge || currentMode == AppMode::DeleteParallelEdge) {
        Edge* foundEdge = findEdgeUnderCursor(currentPos);
        if (foundEdge != hoveredEdge) {
            hoveredEdge = foundEdge;
            requireRedraw = true;
        }
    }

    if (draggedVertex || draggedPolygon) {
        int dx = currentPos.x() - lastMousePos.x();
        int dy = currentPos.y() - lastMousePos.y();

        if (draggedVertex) {
            draggedVertex->point.rx() += dx;
            draggedVertex->point.ry() += dy;
            scene->constraintManager.SolveConstraints(draggedVertex);
        } else if (draggedPolygon) {
            draggedPolygon->translate(dx, dy);
            scene->constraintManager.SolveConstraints(nullptr);
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
            return true;
        }
    }
    return false;
}