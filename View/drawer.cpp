#include "drawer.h"
#include <QImage>
#include <QPainter>
#include "../Models/edge.h"
#include "../Models/vertex.h"
#include "../Models/polygon.h"


Drawer::Drawer(std::vector<Polygon *> list) {
    for(const auto* it : list){
        polygons.push_back(it);
    }
}


void Drawer::Render(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    QPen linePen(Qt::black, 1);
    painter.setPen(linePen);

    for (const auto& poly : polygons) {
        for (const auto& edge : poly->edges) {
            painter.drawLine(edge->vertexes.first->point, edge->vertexes.second->point);
        }
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    for (const auto& poly : polygons) {
        for (const auto& edge : poly->edges) {
            painter.drawEllipse(edge->vertexes.first->point, 3, 3);
        }
    }
}
