#include "window.h"
#include <QPainter>
#include "Models/polygon.h"
#include "Models/vertex.h"
#include "consts.h"

Window::Window(QImage& img) : image{img} {
    setWindowTitle("Polygon Editor");
    resize(1200,1600);

    polygons.push_back(std::make_unique<Polygon>(
        std::initializer_list<QPoint>{QPoint(100, 100), QPoint(300, 100), QPoint(300, 300), QPoint(100, 300)}
        ));

    std::vector<Polygon*> polyPointers;
    for (auto& p : polygons) polyPointers.push_back(p.get());

    drawer = std::make_unique<Drawer>(polyPointers);
}

void Window::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    drawer->Render(painter);
}

void Window::mousePressEvent(QMouseEvent *event) {

    if (event->button() == Qt::RightButton) {
        QPoint p = event->pos();
        const float radiusSq = vertexRadius * vertexRadius;

        auto distSq = [](QPoint a, QPoint b) {
            return std::pow(a.x() - b.x(), 2) + std::pow(a.y() - b.y(), 2);
        };

        for (auto& poly : polygons) {
            if (poly->edges.size() <= 3) continue;

            for (size_t i = 0; i < poly->edges.size(); ++i) {
                auto v = poly->edges[i]->vertexes.first;

                if (distSq(p, v->point) <= radiusSq) {
                    size_t prevIdx = (i == 0) ? poly->edges.size() - 1 : i - 1;

                    auto prevVertex = poly->edges[prevIdx]->vertexes.first;
                    auto nextVertex = poly->edges[i]->vertexes.second;

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

                    update();
                    return;
                }
            }
        }
    }

    if (event->button() == Qt::LeftButton) {

        for(const auto& poly : polygons) {
            for(const auto& edge : poly->edges) {
                const auto& v = edge->vertexes.first;
                if(pow((event->pos().x() - v->point.x()),2) + pow((event->pos().y() - v->point.y()),2) <= vertexRadius * vertexRadius)
                {
                    draggedVertex = v.get();
                    lastMousePos = event->pos();
                    return;
                }
            }
        }

        for (const auto& poly : polygons) {
            if (poly->isClicked(event->pos())) {
                draggedPolygon = poly.get();
                lastMousePos = event->pos();
                return;
            }
        }
    }
}

void Window::mouseMoveEvent(QMouseEvent *event) {
    if (draggedPolygon != nullptr) {
        QPoint currentPos = event->pos();

        int dx = currentPos.x() - lastMousePos.x();
        int dy = currentPos.y() - lastMousePos.y();

        draggedPolygon->translate(dx, dy);

        lastMousePos = currentPos;

        refreshScreen();
        return;
    }
    if(draggedVertex != nullptr) {
        QPoint currentPos = event->pos();

        int dx = currentPos.x() - lastMousePos.x();
        int dy = currentPos.y() - lastMousePos.y();

        draggedVertex->point.rx() += dx;
        draggedVertex->point.ry() += dy;
        lastMousePos = currentPos;

        refreshScreen();
        return;
    }
}

void Window::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;

    QPoint p = event->pos();
    const double edgeTolSq = segmentDistans * segmentDistans;
    const double vertexTolSq = vertexRadius * vertexRadius;

    auto distSq = [](QPoint a, QPoint b) {
        return std::pow(a.x() - b.x(), 2) + std::pow(a.y() - b.y(), 2);
    };

    auto segmentDistSq = [&](QPoint pt, QPoint v1, QPoint v2) {
        double l2 = distSq(v1, v2);
        if (l2 == 0) return distSq(pt, v1);

        double t = ((pt.x() - v1.x()) * (v2.x() - v1.x()) +
                    (pt.y() - v1.y()) * (v2.y() - v1.y())) / l2;
        t = std::max(0.0, std::min(1.0, t));

        QPointF proj(v1.x() + t * (v2.x() - v1.x()),
                     v1.y() + t * (v2.y() - v1.y()));

        return std::pow(pt.x() - proj.x(), 2) + std::pow(pt.y() - proj.y(), 2);
    };

    for (auto& poly : polygons) {
        for (size_t i = 0; i < poly->edges.size(); ++i) {
            auto& edge = poly->edges[i];
            QPoint v1 = edge->vertexes.first->point;
            QPoint v2 = edge->vertexes.second->point;

            if (distSq(p, v1) < vertexTolSq || distSq(p, v2) < vertexTolSq) {
                continue;
            }

            if (segmentDistSq(p, v1, v2) <= edgeTolSq) {
                auto newVertex = std::make_shared<Vertex>(p.x(), p.y());
                auto newEdge1 = std::make_unique<Edge>(edge->vertexes.first, newVertex);
                auto newEdge2 = std::make_unique<Edge>(newVertex, edge->vertexes.second);

                poly->edges.erase(poly->edges.begin() + i);

                // Reverse order insertion to maintain sequence v1 -> newVertex -> v2
                poly->edges.insert(poly->edges.begin() + i, std::move(newEdge2));
                poly->edges.insert(poly->edges.begin() + i, std::move(newEdge1));

                update();
                return;
            }
        }
    }
}


void Window::refreshScreen() {
    QPainter painter(this);
    drawer->Render(painter);
    update();
}


void Window::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        draggedPolygon = nullptr;
        draggedVertex = nullptr;
    }
}

