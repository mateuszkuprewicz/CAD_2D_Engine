#include "drawer.h"
#include "Renderer/CustomGeometryRenderer.h"
#include "Renderer/QtGeometryRenderer.h"
#include <QImage>
#include <QPainter>
#include <QFontMetricsF>
#include "../Models/edge.h"
#include "../Models/vertex.h"
#include "../Models/polygon.h"
#include "../Models/Constraints/lengthconstraint.h"
#include "../Models/Constraints/parallelismconstraint.h"
#include "../Models/Constraints/continuityconstraint.h"
#include "../Models/edgetypes.h"
#include "../Models/scene.h"
#include <unordered_map>
#include <unordered_set>
#include <cmath>

namespace {
// Helper function to format subscript numbers for parallel constraint labels
QString getSubscriptNumber(int num) {
    QString res = QString::number(num);
    QString sub;
    for (QChar c : res) {
        sub += QChar(0x2080 + c.unicode() - '0');
    }
    return sub;
}

struct LabelItem {
    QString text;
    QColor color;
};
}

Drawer::Drawer(Scene *s) {
    scene = s;
    renderer = std::make_unique<QtGeometryRenderer>();
}

void Drawer::Render(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    // Execute separated drawing stages
    DrawPolygons(painter);
    DrawConstraints(painter);
}

void Drawer::DrawPolygons(QPainter &painter) {
    if (!scene) return;

    // Render polygon edges
    for (const auto& poly : scene->Polygons) {
        for (const auto& edge : poly->edges) {
            QPen basePen = (edge.get() == hoveredEdge) ? QPen(Qt::green, 3) : QPen(Qt::black, 1);
            QPen dashedBasePen = (edge.get() == hoveredEdge) ? QPen(Qt::green, 3, Qt::DashLine) : QPen(Qt::lightGray, 1, Qt::DashLine);
            QPen curvePen = QPen(Qt::black, 1);

            if (edge->isBezier()) {
                // Draw base dashed line
                painter.save();
                renderer->drawEdge(painter, edge->vertices.first->point, edge->vertices.second->point, dashedBasePen);
                painter.restore();

                // Draw Bezier curve
                painter.save();
                renderer->drawBezier(painter,
                                     edge->vertices.first->point,
                                     edge->bezier->controlP1->point,
                                     edge->bezier->controlP2->point,
                                     edge->vertices.second->point,
                                     curvePen);

                // Draw control polygon lines
                renderer->drawEdge(painter, edge->vertices.first->point, edge->bezier->controlP1->point, QPen(Qt::gray, 1, Qt::DashLine));
                renderer->drawEdge(painter, edge->bezier->controlP1->point, edge->bezier->controlP2->point, QPen(Qt::gray, 1, Qt::DashLine));
                renderer->drawEdge(painter, edge->bezier->controlP2->point, edge->vertices.second->point, QPen(Qt::gray, 1, Qt::DashLine));

                // Draw control point handles
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::red);
                painter.drawRect(QRectF(edge->bezier->controlP1->point.x() - 4, edge->bezier->controlP1->point.y() - 4, 8, 8));
                painter.drawRect(QRectF(edge->bezier->controlP2->point.x() - 4, edge->bezier->controlP2->point.y() - 4, 8, 8));
                painter.restore();
            }
            else if (edge->isArc()) {
                // Draw arc geometry
                renderer->drawArc(painter, edge->vertices.first->point, edge->vertices.second->point, edge->arc->radius, edge->arc->isMajorArc, curvePen);

                // Draw chord line
                renderer->drawEdge(painter, edge->vertices.first->point, edge->vertices.second->point, dashedBasePen);
            }
            else {
                // Draw standard straight edge
                renderer->drawEdge(painter, edge->vertices.first->point, edge->vertices.second->point, basePen);
            }
        }
    }

    // Render vertices
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    for (const auto& poly : scene->Polygons) {
        for (const auto& edge : poly->edges) {
            painter.drawEllipse(edge->vertices.first->point, 3, 3);
        }
    }
}

void Drawer::DrawConstraints(QPainter &painter) {
    if (!scene) return;

    int parallelIdCounter = 1;
    std::unordered_map<const AbstractConstraint*, int> parallelIds;
    std::unordered_set<const AbstractConstraint*> drawnConstraints;
    std::unordered_map<const Edge*, std::vector<LabelItem>> edgeLabels;

    // Collect constraint labels for edges
    for (auto& [v, list] : scene->constraintManager.Constraints) {
        for (const auto& constraint : list) {
            if (drawnConstraints.find(constraint.get()) != drawnConstraints.end()) continue;
            drawnConstraints.insert(constraint.get());

            if (auto* lenConstraint = dynamic_cast<const LengthConstraint*>(constraint.get())) {
                const auto* edge = lenConstraint->GetEdge();
                if (edge) {
                    QString text = QString::number(edge->GetLength(), 'f', 1);
                    edgeLabels[edge].push_back({text, Qt::blue});
                }
            }
            else if (auto* parConstraint = dynamic_cast<const ParallelismConstraint*>(constraint.get())) {
                if (parallelIds.find(parConstraint) == parallelIds.end()) {
                    parallelIds[parConstraint] = parallelIdCounter++;
                }
                int id = parallelIds[parConstraint];
                QString symbol = "||" + getSubscriptNumber(id);
                if (const auto* e1 = parConstraint->GetEdge1()) {
                    edgeLabels[e1].push_back({symbol, Qt::red});
                }
                if (const auto* e2 = parConstraint->GetEdge2()) {
                    edgeLabels[e2].push_back({symbol, Qt::red});
                }
            }
        }
    }

    // Render edge-aligned text labels
    QFontMetricsF fm(painter.font());
    const qreal spacing = 8.0;

    for (const auto& [edge, items] : edgeLabels) {
        if (!edge || items.empty()) continue;

        QPointF p1 = edge->vertices.first->point;
        QPointF p2 = edge->vertices.second->point;
        QPointF midPoint = (p1 + p2) / 2.0;

        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double angle = std::atan2(dy, dx) * 180.0 / M_PI;

        if (angle > 90.0) angle -= 180.0;
        else if (angle < -90.0) angle += 180.0;

        qreal totalWidth = 0.0;
        for (size_t i = 0; i < items.size(); ++i) {
            totalWidth += fm.horizontalAdvance(items[i].text);
            if (i > 0) totalWidth += spacing;
        }

        painter.save();
        painter.translate(midPoint);
        painter.rotate(angle);

        qreal currentX = -totalWidth / 2.0;
        for (const auto& item : items) {
            painter.setPen(item.color);
            painter.drawText(QPointF(currentX, -5), item.text);
            currentX += fm.horizontalAdvance(item.text) + spacing;
        }
        painter.restore();
    }

    // Render continuity constraints labels
    painter.setPen(QPen(Qt::darkMagenta));
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);

    std::unordered_set<const Vertex*> drawnContVertices;
    for (const auto& [v, list] : scene->constraintManager.Constraints) {
        for (const auto& constraint : list) {
            if (auto* contConstraint = dynamic_cast<const ContinuityConstraint*>(constraint.get())) {
                Vertex* sharedV = contConstraint->getVertex();

                if (drawnContVertices.find(sharedV) != drawnContVertices.end()) continue;
                drawnContVertices.insert(sharedV);

                QString label;
                if (dynamic_cast<const G1ContinuityConstraint*>(contConstraint)) label = "G1";
                else if (dynamic_cast<const C1ContinuityConstraint*>(contConstraint)) label = "C1";

                if (!label.isEmpty()) {
                    painter.drawText(sharedV->point.x() + 8, sharedV->point.y() - 8, label);
                }
            }
        }
    }
}

void Drawer::SetHoveredEdge(const Edge * e) {
    hoveredEdge = e;
}