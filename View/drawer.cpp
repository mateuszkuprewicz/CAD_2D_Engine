#include "drawer.h"
#include <QImage>
#include <QPainter>
#include "../Models/edge.h"
#include "../Models/vertex.h"
#include "../Models/polygon.h"
#include "../Models/Constraints/lengthconstraint.h"
#include "../Models/Constraints/parallelismconstraint.h"
#include <unordered_map>
#include <unordered_set>

Drawer::Drawer(Scene *s) {
    scene = s;
}

QString getSubscriptNumber(int num) {
    QString res = QString::number(num);
    QString sub;
    for(QChar c : res) {
        sub += QChar(0x2080 + c.unicode() - '0');
    }
    return sub;
}

struct LabelItem {
    QString text;
    QColor color;
};

void Drawer::Render(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    // --- 1. Draw Edges ---
    for (const auto& poly : scene->Polygons) {
        for (const auto& edge : poly->edges) {
            if (edge.get() == hoveredEdge) {
                painter.setPen(QPen(Qt::green, 3));
            } else {
                painter.setPen(QPen(Qt::black, 1));
            }
            painter.drawLine(edge->vertices.first->point, edge->vertices.second->point);
        }
    }

    // --- 2. Draw Vertices ---
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    for (const auto& poly : scene->Polygons) {
        for (const auto& edge : poly->edges) {
            painter.drawEllipse(edge->vertices.first->point, 3, 3);
        }
    }

    // --- 3. Collect Labels Per Edge ---
    int parallelIdCounter = 1;
    std::unordered_map<const AbstractConstraint*, int> parallelIds;
    std::unordered_set<const AbstractConstraint*> drawnConstraints;

    std::unordered_map<const Edge*, std::vector<LabelItem>> edgeLabels;

    for (auto& [v, list] : scene->constraintManager.Constraints) {
        for(const auto& constraint : list) {
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

    // --- 4. Render Aggregated Edge Labels Side-By-Side ---
    QFontMetricsF fm(painter.font());
    const qreal spacing = 8.0; // Horizontal gap between labels

    for (const auto& [edge, items] : edgeLabels) {
        if (!edge || items.empty()) continue;

        QPointF p1 = edge->vertices.first->point;
        QPointF p2 = edge->vertices.second->point;
        QPointF midPoint = (p1 + p2) / 2.0;

        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double angle = std::atan2(dy, dx) * 180.0 / M_PI;

        // Keep text right-side up for readability
        if (angle > 90.0) angle -= 180.0;
        else if (angle < -90.0) angle += 180.0;

        // Calculate total width of all labels combined plus spacing
        qreal totalWidth = 0.0;
        for (size_t i = 0; i < items.size(); ++i) {
            totalWidth += fm.horizontalAdvance(items[i].text);
            if (i > 0) totalWidth += spacing;
        }

        painter.save();
        painter.translate(midPoint);
        painter.rotate(angle);

        // Draw each label sequentially starting from the centered position
        qreal currentX = -totalWidth / 2.0;
        for (const auto& item : items) {
            painter.setPen(item.color);
            painter.drawText(QPointF(currentX, -5), item.text);
            currentX += fm.horizontalAdvance(item.text) + spacing;
        }
        painter.restore();
    }
}