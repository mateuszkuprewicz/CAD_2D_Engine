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
#include "../Models/Constraints/continuityconstraint.h"
#include "../Models/edgetypes.h"
#include "../Models/continuityconstraintmanager.h"

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
            if (segmentDistSq(p, v1->point, v2->point) <= ClickArea::segmentDistans * ClickArea::segmentDistans) {
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

            if (distSq(p, v1) < ClickArea::vertexRadius * ClickArea::vertexRadius || distSq(p, v2) < ClickArea::vertexRadius * ClickArea::vertexRadius) {
                continue;
            }

            if (segmentDistSq(p, v1, v2) <= ClickArea::segmentDistans * ClickArea::segmentDistans) {
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
    const float radiusSq = ClickArea::vertexRadius * ClickArea::vertexRadius;

    for (const auto& poly : scene->Polygons) {

        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;

            if (distSq(p, v1->point) <= radiusSq) {
                QMenu vMenu(parent);
                QAction* delVertexAction = vMenu.addAction("Delete Vertex");
                delVertexAction->setEnabled(poly->edges.size() > 3);

                size_t prevIdx = (i == 0) ? poly->edges.size() - 1 : i - 1;
                auto prevEdge = poly->edges[prevIdx].get();
                auto currEdge = poly->edges[i].get();

                bool hasArc = prevEdge->isArc() || currEdge->isArc();
                bool hasCurve = prevEdge->isBezier() || currEdge->isBezier() || hasArc;

                QMenu* contMenu = nullptr;
                QAction* setG0 = nullptr;
                QAction* setG1 = nullptr;
                QAction* setC1 = nullptr;

                if (hasCurve) {
                    contMenu = vMenu.addMenu("Continuity");
                    setG0 = contMenu->addAction("G0 (None)");
                    setG1 = contMenu->addAction("G1");
                    setC1 = contMenu->addAction("C1");

                    if (hasArc) {
                        setC1->setEnabled(false);
                    }
                }

                QAction* sel = vMenu.exec(event->globalPos());

                if (sel == delVertexAction) {
                    Vertex* v0 = prevEdge->vertices.first.get() == v1.get() ? prevEdge->vertices.second.get() : prevEdge->vertices.first.get();
                    Vertex* v2 = currEdge->vertices.first.get() == v1.get() ? currEdge->vertices.second.get() : currEdge->vertices.first.get();

                    auto state0 = ContinuityConstraintManager::SaveState(&scene->constraintManager, v0);
                    auto state2 = ContinuityConstraintManager::SaveState(&scene->constraintManager, v2);

                    scene->constraintManager.DelEdgeConstraints(prevEdge->vertices.first.get(), prevEdge->vertices.second.get());
                    scene->constraintManager.DelEdgeConstraints(currEdge->vertices.first.get(), currEdge->vertices.second.get());

                    scene->delVertex(poly.get(), v1.get(), prevIdx, i);

                    ContinuityConstraintManager::RestoreState(scene, poly.get(), v0, state0);
                    ContinuityConstraintManager::RestoreState(scene, poly.get(), v2, state2);

                    return true;
                } else if (sel && hasCurve) {
                    scene->constraintManager.DelContinuityConstraint(v1.get());
                    if (sel == setG1) {
                        scene->AddConstraint(std::make_shared<G1ContinuityConstraint>(v1.get(), prevEdge, currEdge));
                        scene->TrySolveConstraints(v1.get(), nullptr);
                    } else if (sel == setC1) {
                        scene->AddConstraint(std::make_shared<C1ContinuityConstraint>(v1.get(), prevEdge, currEdge));
                        scene->TrySolveConstraints(v1.get(), nullptr);
                    }
                    return true;
                }
                return true;
            }
        }

        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto v1 = poly->edges[i]->vertices.first;
            auto v2 = poly->edges[i]->vertices.second;

            if (segmentDistSq(p, v1->point, v2->point) <= ClickArea::segmentDistans * ClickArea::segmentDistans) {
            //edge clicked
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

                QMenu* curveMenu = menu.addMenu("Edge Type");
                QAction* convertToBezier = curveMenu->addAction("Convert to Bezier Curve");
                QAction* convertToArc = curveMenu->addAction("Convert to Circular Arc");
                QAction* convertToLine = curveMenu->addAction("Convert to Line");

                convertToBezier->setEnabled(!poly->edges[i]->isBezier());
                convertToLine->setEnabled(poly->edges[i]->isBezier() || poly->edges[i]->isArc());

                QAction* selectedAction = menu.exec(event->globalPos());

                if (selectedAction == addLengthAction) {
                    bool ok = false;
                    float current_length = poly->edges[i]->GetLength();
                    double new_length = QInputDialog::getDouble(parent, "Edge Length Constraint", "Enter edge length:", current_length, 1.0, 10000.0, 1, &ok);

                    if (ok) {
                        auto constraint = std::make_shared<LengthConstraint>(poly->edges[i].get(), new_length);
                        scene->AddConstraint(constraint);
                        scene->TrySolveConstraints(poly->edges[i]->vertices.first.get(), poly->edges[i]->vertices.second.get());
                        return true;
                    }
                }
                else if (selectedAction == delLengthAction) {
                    auto* c = scene->constraintManager.GetLengthConstraint(v1.get(), v2.get());
                    if (c) {
                        scene->DelConstraint(c);
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
                else if (selectedAction == convertToBezier) {
                    size_t prevIdx = (i == 0) ? poly->edges.size() - 1 : i - 1;
                    size_t nextIdx = (i + 1) % poly->edges.size();

                    auto surviving = scene->constraintManager.HandleEdgeConversion(poly->edges[i].get(), poly->edges[prevIdx].get(), poly->edges[nextIdx].get());

                    auto edge = poly->edges[i].get();
                    edge->bezier = std::make_unique<BezierCurve>(v1->point, v2->point);
                    edge->arc.reset();

                    for (const auto& c : surviving) scene->AddConstraint(c);

                    scene->AddConstraint(std::make_shared<G1ContinuityConstraint>(v1.get(), poly->edges[prevIdx].get(), edge));
                    scene->AddConstraint(std::make_shared<G1ContinuityConstraint>(v2.get(), edge, poly->edges[nextIdx].get()));
                    scene->TrySolveConstraints(v1.get(), v2.get());
                    return true;
                }
                else if (selectedAction == convertToArc) {
                    size_t prevIdx = (i == 0) ? poly->edges.size() - 1 : i - 1;
                    size_t nextIdx = (i + 1) % poly->edges.size();

                    auto surviving = scene->constraintManager.HandleEdgeConversion(poly->edges[i].get(), poly->edges[prevIdx].get(), poly->edges[nextIdx].get());

                    auto edge = poly->edges[i].get();
                    edge->arc = std::make_unique<ArcCurve>(v1->point, v2->point);
                    edge->bezier.reset();

                    for (const auto& c : surviving) scene->AddConstraint(c);

                    scene->TrySolveConstraints(v1.get(), v2.get());
                    return true;
                }
                else if (selectedAction == convertToLine) {
                    size_t prevIdx = (i == 0) ? poly->edges.size() - 1 : i - 1;
                    size_t nextIdx = (i + 1) % poly->edges.size();

                    auto surviving = scene->constraintManager.HandleEdgeConversion(poly->edges[i].get(), poly->edges[prevIdx].get(), poly->edges[nextIdx].get());

                    auto edge = poly->edges[i].get();
                    edge->bezier.reset();
                    edge->arc.reset();

                    for (const auto& c : surviving) scene->AddConstraint(c);

                    scene->TrySolveConstraints(v1.get(), v2.get());
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
                auto constraintOpt = std::make_shared<ParallelismConstraint>(firstParallelEdge, hoveredEdge);
                scene->AddConstraint(std::move(constraintOpt));
                scene->TrySolveConstraints(firstParallelEdge->vertices.first.get(), firstParallelEdge->vertices.second.get());
            } else {
                auto* c = scene->constraintManager.GetParallelConstraint(firstParallelEdge, hoveredEdge);
                if (c) {
                    scene->DelConstraint(c);
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
    const float radiusSq = ClickArea::vertexRadius * ClickArea::vertexRadius;

    for (const auto& poly : scene->Polygons) {
        for (const auto& edge : poly->edges) {
            if (edge->isBezier()) {
                if (distSq(p, edge->bezier->controlP1->point) <= radiusSq) {
                    lastMousePos = p;
                    draggedVertex = edge->bezier->controlP1.get();
                    return true;
                }
                if (distSq(p, edge->bezier->controlP2->point) <= radiusSq) {
                    lastMousePos = p;
                    draggedVertex = edge->bezier->controlP2.get();
                    return true;
                }
            }
        }
    }

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

            for (const auto& poly : scene->Polygons) {
                for (const auto& edge : poly->edges) {
                    if (edge->isBezier()) {
                        if (edge->vertices.first.get() == draggedVertex) {
                            edge->bezier->controlP1->point.rx() += dx;
                            edge->bezier->controlP1->point.ry() += dy;
                        }
                        if (edge->vertices.second.get() == draggedVertex) {
                            edge->bezier->controlP2->point.rx() += dx;
                            edge->bezier->controlP2->point.ry() += dy;
                        }
                    }
                }
            }

            scene->SolveConstraints(draggedVertex);
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
            return true;
        }
    }
    return false;
}