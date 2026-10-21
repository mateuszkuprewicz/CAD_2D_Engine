#pragma once
#include <QWidget>
#include <QMouseEvent>
#include <vector>
#include <memory>
#include "../View/drawer.h"
#include "../Models/polygon.h"


class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QImage&);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    std::vector<std::unique_ptr<Polygon>> polygons;
    std::unique_ptr<Drawer> drawer;
    std::vector<Polygon*> getPolygonPointers() const;

    Polygon* draggedPolygon = nullptr;
    Vertex* draggedVertex = nullptr;
    QPoint lastMousePos;

    void refreshScreen();
};
