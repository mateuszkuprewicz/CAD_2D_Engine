#include "window.h"
#include <QPainter>

Window::Window(QImage& img) {
    setWindowTitle("Polygon Editor");
    resize(1200, 1600);
    setMouseTracking(true); // Enables mouseMoveEvent without clicking

    polygons.push_back(std::make_unique<Polygon>(
        std::initializer_list<QPoint>{QPoint(100, 100), QPoint(300, 100), QPoint(300, 300), QPoint(100, 300)}
        ));

    std::vector<Polygon*> polyPointers;
    for (auto& p : polygons) polyPointers.push_back(p.get());

    drawer = std::make_unique<Drawer>(polyPointers);
    inputHandler = std::make_unique<InputHandler>();
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

    // Pass hovered edge to drawer immediately before rendering
    drawer->SetHoveredEdge(inputHandler->getHoveredEdge());
    drawer->Render(painter);
}

void Window::mousePressEvent(QMouseEvent *event) {
    bool requireRedraw = false;

    if (event->button() == Qt::RightButton) {
        requireRedraw = inputHandler->handleRightClick(this, event, getPolygonPointers());
    } else if (event->button() == Qt::LeftButton) {
        requireRedraw = inputHandler->handleLeftClick(event, getPolygonPointers());
    }

    if (requireRedraw) {
        update();
    }
}

void Window::mouseMoveEvent(QMouseEvent *event) {
    if (inputHandler->handleMouseMove(event, getPolygonPointers())) {
        update();
    }
}

void Window::mouseDoubleClickEvent(QMouseEvent *event) {
    if (inputHandler->handleDoubleClick(event, getPolygonPointers())) {
        update();
    }
}

void Window::mouseReleaseEvent(QMouseEvent *event) {
    if (inputHandler->handleMouseRelease(event)) {
        update();
    }
}