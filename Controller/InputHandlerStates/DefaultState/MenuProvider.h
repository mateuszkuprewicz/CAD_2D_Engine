#pragma once
#include <QPoint>

class QWidget;

enum class EdgeMenuAction {
    None,
    AddLength,
    DeleteLength,
    AddParallel,
    DeleteParallel,
    ConvertToBezier,
    ConvertToArc,
    ConvertToLine
};

enum class VertexMenuAction {
    None,
    setG0,
    setG1,
    setC1,
    delVertexAction
};

class MenuProvider
{
public:
    EdgeMenuAction execEdgeMenu(QWidget* parent,
                                const QPoint& globalPos,
                                bool hasLength,
                                bool hasParallel,
                                bool isBezier,
                                bool isArc) const;

    VertexMenuAction execVertexMenu(QWidget* parent,
                                    const QPoint& globalPos,
                                    bool canDelete,
                                    bool hasCurve,
                                    bool hasArc) const;
};