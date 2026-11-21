#include "./menuprovider.h"
#include <QMenu>
#include <QAction>
#include <QWidget>

EdgeMenuAction MenuProvider::execEdgeMenu(QWidget* parent,
                                          const QPoint& globalPos,
                                          bool hasLength,
                                          bool hasParallel,
                                          bool isBezier,
                                          bool isArc) const {
    QMenu menu(parent);

    QMenu* lengthMenu = menu.addMenu("Length Constraint");
    QAction* addLengthAction = lengthMenu->addAction("Add / Edit");
    QAction* delLengthAction = lengthMenu->addAction("Delete");
    delLengthAction->setEnabled(hasLength);

    QMenu* parallelMenu = menu.addMenu("Parallelism Constraint");
    QAction* addParallelAction = parallelMenu->addAction("Add");
    QAction* delParallelAction = parallelMenu->addAction("Delete");
    delParallelAction->setEnabled(hasParallel);

    QMenu* curveMenu = menu.addMenu("Edge Type");
    QAction* convertToBezier = curveMenu->addAction("Convert to Bezier Curve");
    QAction* convertToArc = curveMenu->addAction("Convert to Circular Arc");
    QAction* convertToLine = curveMenu->addAction("Convert to Line");

    convertToBezier->setEnabled(!isBezier);
    convertToLine->setEnabled(isBezier || isArc);

    QAction* selected = menu.exec(globalPos);

    if (selected == addLengthAction) return EdgeMenuAction::AddLength;
    if (selected == delLengthAction) return EdgeMenuAction::DeleteLength;
    if (selected == addParallelAction) return EdgeMenuAction::AddParallel;
    if (selected == delParallelAction) return EdgeMenuAction::DeleteParallel;
    if (selected == convertToBezier) return EdgeMenuAction::ConvertToBezier;
    if (selected == convertToArc) return EdgeMenuAction::ConvertToArc;
    if (selected == convertToLine) return EdgeMenuAction::ConvertToLine;

    return EdgeMenuAction::None;
}

VertexMenuAction MenuProvider::execVertexMenu(QWidget* parent,
                                              const QPoint& globalPos,
                                              bool canDelete,
                                              bool hasCurve,
                                              bool hasArc) const {
    QMenu vMenu(parent);
    QAction* delVertexAction = vMenu.addAction("Delete Vertex");
    delVertexAction->setEnabled(canDelete);

    QAction* setG0 = nullptr;
    QAction* setG1 = nullptr;
    QAction* setC1 = nullptr;

    if (hasCurve) {
        QMenu* contMenu = vMenu.addMenu("Continuity");
        setG0 = contMenu->addAction("G0 (None)");
        setG1 = contMenu->addAction("G1");
        setC1 = contMenu->addAction("C1");

        if (hasArc) {
            setC1->setEnabled(false);
        }
    }

    QAction* selected = vMenu.exec(globalPos);

    if (selected == delVertexAction) return VertexMenuAction::delVertexAction;
    if (selected && selected == setG0) return VertexMenuAction::setG0;
    if (selected && selected == setG1) return VertexMenuAction::setG1;
    if (selected && selected == setC1) return VertexMenuAction::setC1;

    return VertexMenuAction::None;
}