#pragma once
#include "edge.h"
#include <vector>
#include <memory>
#include <QPoint>
#include "Constraints/abstractconstraint.h"

class Polygon
{
public:
    Polygon(std::initializer_list<QPoint>);
    std::vector<std::unique_ptr<Edge>> edges;
    void translate(int dx, int dy);
    bool isClicked(QPoint clickPos) const;
    void AddConstraint(std::unique_ptr<AbstractConstraint>);
    std::vector<const AbstractConstraint*> Constraints() const;
private:
    std::vector<std::unique_ptr<AbstractConstraint>> constraints;
};

