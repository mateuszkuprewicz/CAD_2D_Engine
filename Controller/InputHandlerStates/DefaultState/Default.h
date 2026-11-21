#pragma once
#include "../AbstractInputHandlerState.h"
#include "MenuProvider.h"

class Default : public AbstractInputHandlerState
{
public:
    Default(InputHandler* context, Scene* scene);

    bool handleDoubleClick(QMouseEvent* event) override;
    bool handleRightClick(QWidget* parent, QMouseEvent* event) override;
    bool handleLeftClick(QMouseEvent* event) override;

private:
    MenuProvider menuProvider;
};