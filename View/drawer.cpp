#include "drawer.h"
#include <QImage>
#include <QPainter>
#include "../Models/edge.h"
#include "../Models/vertex.h"
#include "../Models/polygon.h"


Drawer::Drawer(QImage& img, std::initializer_list<Polygon *> list) : image{img} {
    img.fill(Qt::white);
    for(auto it = list.begin(); it != list.end(); ++it){
        polygons.push_back(*it);
    }
}

void Drawer::drawEdge(const Edge& edge){
    QPainter painter(&image);
    QPen pointPen(Qt::black, 8, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pointPen);
    painter.drawPoint(edge.vertexes.first->point);

    QPen linePen(Qt::black, 4);
    painter.setPen(linePen);
    painter.drawLine(edge.vertexes.first->point, edge.vertexes.second->point);
}

void Drawer::Render(){
    for(auto vec_it = polygons.begin(); vec_it != polygons.end(); ++vec_it){
        for(auto it = (*vec_it)->edges.begin(); it != (*vec_it)->edges.end(); ++it){
            drawEdge((**it));
        }
    }
}
