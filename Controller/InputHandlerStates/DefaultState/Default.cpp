#include "./default.h"
#include "../../inputhandler.h"
#include "../DraggingVertexState/DraggingVertex.h"
#include "../DraggingPolygonState/DraggingPolygon.h"
#include "../ParallelEdgesStates/selectingparalleledge.h"
#include "../ParallelEdgesStates/DeletingParallelEdge.h"
#include "../../../Models/scene.h"
#include "../../../Models/polygon.h"
#include "../../../Models/edge.h"
#include "../../../Models/vertex.h"
#include "../../../Models/edgetypes.h"
#include "../../../Models/Constraints/lengthconstraint.h"
#include "../../../Models/Constraints/parallelismconstraint.h"
#include "../../../Models/Constraints/continuityconstraint.h"
#include "../../../Models/continuityconstraintmanager.h"
#include <QInputDialog>
#include <QMouseEvent>
#include <memory>

Default::Default(InputHandler* context, Scene* scene)
    : AbstractInputHandlerState(context, scene) {}

bool Default::handleDoubleClick(QMouseEvent* event) {
    if (!scene || event->button() != Qt::LeftButton) return false;

    const QPointF p = event->position();
    Polygon* poly = nullptr;
    size_t edgeIdx = 0;

    if (context->getHitTest().EdgeHitTest(scene, p, true, &poly, &edgeIdx)) {
        auto newVertex = std::make_shared<Vertex>(p.x(), p.y());
        scene->addVertex(poly, std::move(newVertex), edgeIdx);
        return true;
    }

    return false;
}

bool Default::handleRightClick(QWidget* parent, QMouseEvent* event) {
    if (!scene || event->button() != Qt::RightButton) return false;

    const QPoint p = event->pos();
    Polygon* poly = nullptr;
    size_t edgeIdx = 0;

    if (Vertex* v1 = context->getHitTest().VertexHitTest(scene, p, false, &poly, &edgeIdx)) {
        const size_t prevIdx = (edgeIdx == 0) ? poly->edges.size() - 1 : edgeIdx - 1;
        Edge* prevEdge = poly->edges[prevIdx].get();
        Edge* currEdge = poly->edges[edgeIdx].get();

        const bool hasArc = prevEdge->isArc() || currEdge->isArc();
        const bool hasCurve = prevEdge->isBezier() || currEdge->isBezier() || hasArc;

        const VertexMenuAction action = menuProvider.execVertexMenu(
            parent, event->globalPos(), poly->edges.size() > 3, hasCurve, hasArc);

        switch (action) {
        case VertexMenuAction::delVertexAction: {
            Vertex* v0 = (prevEdge->vertices.first.get() == v1)
                             ? prevEdge->vertices.second.get()
                             : prevEdge->vertices.first.get();
            Vertex* v2 = (currEdge->vertices.first.get() == v1)
                             ? currEdge->vertices.second.get()
                             : currEdge->vertices.first.get();

            auto state0 = ContinuityConstraintManager::SaveState(&scene->constraintManager, v0);
            auto state2 = ContinuityConstraintManager::SaveState(&scene->constraintManager, v2);

            scene->constraintManager.DelEdgeConstraints(prevEdge->vertices.first.get(), prevEdge->vertices.second.get());
            scene->constraintManager.DelEdgeConstraints(currEdge->vertices.first.get(), currEdge->vertices.second.get());

            scene->delVertex(poly, v1, prevIdx, edgeIdx);

            ContinuityConstraintManager::RestoreState(scene, poly, v0, state0);
            ContinuityConstraintManager::RestoreState(scene, poly, v2, state2);
            return true;
        }
        case VertexMenuAction::setG0:
            scene->constraintManager.DelContinuityConstraint(v1);
            return true;
        case VertexMenuAction::setG1:
            scene->constraintManager.DelContinuityConstraint(v1);
            scene->AddConstraint(std::make_shared<G1ContinuityConstraint>(v1, prevEdge, currEdge));
            scene->TrySolveConstraints(v1, nullptr);
            return true;
        case VertexMenuAction::setC1:
            scene->constraintManager.DelContinuityConstraint(v1);
            scene->AddConstraint(std::make_shared<C1ContinuityConstraint>(v1, prevEdge, currEdge));
            scene->TrySolveConstraints(v1, nullptr);
            return true;
        case VertexMenuAction::None:
            return true;
        }
    }

    if (Edge* edge = context->getHitTest().EdgeHitTest(scene, p, false, &poly, &edgeIdx)) {
        Vertex* v1 = edge->vertices.first.get();
        Vertex* v2 = edge->vertices.second.get();

        const bool hasLength = scene->constraintManager.HasEdgeConstraint<LengthConstraint>(v1, v2);
        const bool hasParallel = scene->constraintManager.HasEdgeConstraint<ParallelismConstraint>(v1, v2);

        const EdgeMenuAction action = menuProvider.execEdgeMenu(
            parent, event->globalPos(), hasLength, hasParallel, edge->isBezier(), edge->isArc());

        switch (action) {
        case EdgeMenuAction::AddLength: {
            bool ok = false;
            const float currentLength = edge->GetLength();
            const double newLength = QInputDialog::getDouble(
                parent, "Edge Length Constraint", "Enter edge length:", currentLength, 1.0, 10000.0, 1, &ok);

            if (ok) {
                auto constraint = std::make_shared<LengthConstraint>(edge, newLength);
                scene->AddConstraint(std::move(constraint));
                scene->TrySolveConstraints(v1, v2);
                return true;
            }
            return false;
        }
        case EdgeMenuAction::DeleteLength: {
            if (auto* c = scene->constraintManager.GetLengthConstraint(v1, v2)) {
                scene->DelConstraint(c);
            }
            return true;
        }
        case EdgeMenuAction::AddParallel:
            context->ChangeState(std::make_unique<SelectingParallelEdge>(context, scene, edge, poly));
            return true;
        case EdgeMenuAction::DeleteParallel:
            context->ChangeState(std::make_unique<DeletingParallelEdge>(context, scene, edge, poly));
            return true;
        case EdgeMenuAction::ConvertToBezier: {
            const size_t prevIdx = (edgeIdx == 0) ? poly->edges.size() - 1 : edgeIdx - 1;
            const size_t nextIdx = (edgeIdx + 1) % poly->edges.size();

            auto surviving = scene->constraintManager.HandleEdgeConversion(
                edge, poly->edges[prevIdx].get(), poly->edges[nextIdx].get());

            edge->bezier = std::make_unique<BezierCurve>(v1->point, v2->point);
            edge->arc.reset();

            for (const auto& c : surviving) scene->AddConstraint(c);

            scene->AddConstraint(std::make_shared<G1ContinuityConstraint>(v1, poly->edges[prevIdx].get(), edge));
            scene->AddConstraint(std::make_shared<G1ContinuityConstraint>(v2, edge, poly->edges[nextIdx].get()));
            scene->TrySolveConstraints(v1, v2);
            return true;
        }
        case EdgeMenuAction::ConvertToArc: {
            const size_t prevIdx = (edgeIdx == 0) ? poly->edges.size() - 1 : edgeIdx - 1;
            const size_t nextIdx = (edgeIdx + 1) % poly->edges.size();

            auto surviving = scene->constraintManager.HandleEdgeConversion(
                edge, poly->edges[prevIdx].get(), poly->edges[nextIdx].get());

            edge->arc = std::make_unique<ArcCurve>(v1->point, v2->point);
            edge->bezier.reset();

            for (const auto& c : surviving) scene->AddConstraint(c);

            scene->TrySolveConstraints(v1, v2);
            return true;
        }
        case EdgeMenuAction::ConvertToLine: {
            const size_t prevIdx = (edgeIdx == 0) ? poly->edges.size() - 1 : edgeIdx - 1;
            const size_t nextIdx = (edgeIdx + 1) % poly->edges.size();

            auto surviving = scene->constraintManager.HandleEdgeConversion(
                edge, poly->edges[prevIdx].get(), poly->edges[nextIdx].get());

            edge->bezier.reset();
            edge->arc.reset();

            for (const auto& c : surviving) scene->AddConstraint(c);

            scene->TrySolveConstraints(v1, v2);
            return true;
        }
        case EdgeMenuAction::None:
            return false;
        }
    }

    return false;
}

bool Default::handleLeftClick(QMouseEvent* event) {
    if (!scene || event->button() != Qt::LeftButton) return false;

    const QPoint p = event->pos();

    if (Vertex* v = context->getHitTest().VertexHitTest(scene, p, true)) {
        context->ChangeState(std::make_unique<DraggingVertex>(context, scene, v, p));
        return true;
    }

    for (const auto& poly : scene->Polygons) {
        if (poly->isClicked(p)) {
            context->ChangeState(std::make_unique<DraggingPolygon>(context, scene, poly.get(), p));
            return true;
        }
    }

    return false;
}