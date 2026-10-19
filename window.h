#pragma once
#include <QWidget>

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QImage&);
    void paintEvent(QPaintEvent *event) override;

private:
    QImage &image;
};


