#include "window.h"
#include <QPainter>
#include "../Models/polygon.h"
#include "inputhandler.h"

Window::Window(QImage& img) {
    setWindowTitle("Polygon Editor");
    resize(1200,1600);

    polygons.push_back(std::make_unique<Polygon>(
        std::initializer_list<QPoint>{QPoint(100, 100), QPoint(300, 100), QPoint(300, 300), QPoint(100, 300)}
        ));

    std::vector<Polygon*> polyPointers;
    for (auto& p : polygons) polyPointers.push_back(p.get());

    drawer = std::make_unique<Drawer>(polyPointers);
}

std::vector<Polygon*> Window::getPolygonPointers() const {
    std::vector<Polygon*> rawPointers;
    rawPointers.reserve(polygons.size());
    for (const auto& poly : polygons) {
        rawPointers.push_back(poly.get());
    }
    return rawPointers;
}

void Window::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    drawer->Render(painter);
}

void Window::mousePressEvent(QMouseEvent *event) {
    auto polyPointers = getPolygonPointers();

    if (event->button() == Qt::RightButton) {
        if (InputHandler::handleRightClick(this, event, polyPointers)) {
            update();
        }
    } else if (event->button() == Qt::LeftButton) {
        const auto& temp = InputHandler::handleLeftClick(event, polyPointers, lastMousePos);
        draggedPolygon = temp.first;
        draggedVertex = temp.second;
    }
}

void Window::mouseMoveEvent(QMouseEvent *event) {
    if (InputHandler::handleMouseMove(event, getPolygonPointers(),  draggedPolygon, draggedVertex, lastMousePos)) {
        update();
    }
}

void Window::mouseDoubleClickEvent(QMouseEvent *event) {
    InputHandler::handleDoubleClick(event, getPolygonPointers());
    update();
}

void Window::mouseReleaseEvent(QMouseEvent *event) {
    InputHandler::handleMouseRelease(event, draggedPolygon, draggedVertex);
}