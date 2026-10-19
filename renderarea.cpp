#include "renderarea.h"
#include <QPainter>
#include <QMouseEvent>

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    m_point = QPoint(100,100);
    m_isDragging = false;
    m_pointRadius = 10;
}

void RenderArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::red);
    painter.drawEllipse(m_point, m_pointRadius, m_pointRadius);
}

void RenderArea::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        int dx = event->pos().x() - m_point.x();
        int dy = event->pos().y() - m_point.y();
        if(dx*dx + dy*dy < m_pointRadius * m_pointRadius)
            m_isDragging = true;
    }
}

void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isDragging){
        m_point = event->pos();
        update();
    }
}

void RenderArea::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton){
        m_isDragging = false;
    }
}

void RenderArea::resetPoint()
{
    m_point = QPoint(100,100);
    update();
}
















