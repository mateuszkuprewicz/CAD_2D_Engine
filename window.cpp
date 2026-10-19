#include "window.h"
#include <QPainter>

Window::Window(QImage& img) : image{img} {
    setWindowTitle("Polygon Editor");
    resize(1200,1600);
}

void Window::paintEvent(QPaintEvent *event){
    QPainter painter(this);

    if (!image.isNull()) {
        painter.drawImage(0, 0, image);
    }
}