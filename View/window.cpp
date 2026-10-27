#include "window.h"
#include <QPainter>
#include "Renderer/customgeometryrenderer.h"
#include "Renderer/QtGeometryRenderer.h"

Window::Window(std::unique_ptr<Drawer>&& d, std::unique_ptr<InputHandler>&& i) {
    setWindowTitle("Polygon Editor");
    resize(1200, 1600);
    setMouseTracking(true);
    drawer = std::move(d);
    inputHandler = std::move(i);

    toggleRendererButton = new QPushButton("Now: Qt Renderer", this);
    toggleRendererButton->setGeometry(10, 10, 250, 50);

    toggleRendererButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #1e1e2e;"
        "  color: #cdd6f4;"
        "  border: 1px solid #45475a;"
        "  border-radius: 8px;"
        "  font-family: 'Segoe UI', Arial, sans-serif;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #313244;"
        "  border-color: #89b4fa;"
        "  color: #ffffff;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #45475a;"
        "  border-color: #b4befe;"
        "}"
        );

    toggleRendererButton->raise();
    toggleRendererButton->show();

    connect(toggleRendererButton, &QPushButton::clicked, this, &Window::toggleRenderer);
}

void Window::toggleRenderer() {
    usingCustomRenderer = !usingCustomRenderer;

    if (usingCustomRenderer) {
        drawer->setRenderer(std::make_unique<CustomGeometryRenderer>());
        toggleRendererButton->setText("Now: Custom Renderer ");
    } else {
        drawer->setRenderer(std::make_unique<QtGeometryRenderer>());
        toggleRendererButton->setText("Now: Qt Renderer");
    }
    update();
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