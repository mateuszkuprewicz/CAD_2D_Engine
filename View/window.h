#pragma once
#include <QWidget>
#include <QMouseEvent>
#include <memory>
#include "drawer.h"
#include "../Controller/inputhandler.h"

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(std::unique_ptr<Drawer>&&, std::unique_ptr<InputHandler>&&);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    std::unique_ptr<Drawer> drawer;
    std::unique_ptr<InputHandler> inputHandler;
};