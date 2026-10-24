#include "window.h"
#include <QPainter>

Window::Window(std::unique_ptr<Drawer>&& d, std::unique_ptr<InputHandler>&& i) {
    setWindowTitle("Polygon Editor");
    resize(1200, 1600);
    setMouseTracking(true); // Enables mouseMoveEvent without clicking
    drawer = std::move(d);
    inputHandler = std::move(i);
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
        requireRedraw = inputHandler->handleRightClick(this, event);
    } else if (event->button() == Qt::LeftButton) {
        requireRedraw = inputHandler->handleLeftClick(event);
    }

    if (requireRedraw) {
        update();
    }
}

void Window::mouseMoveEvent(QMouseEvent *event) {
    if (inputHandler->handleMouseMove(event)) {
        update();
    }
}

void Window::mouseDoubleClickEvent(QMouseEvent *event) {
    if (inputHandler->handleDoubleClick(event)) {
        update();
    }
}

void Window::mouseReleaseEvent(QMouseEvent *event) {
    if (inputHandler->handleMouseRelease(event)) {
        update();
    }
}