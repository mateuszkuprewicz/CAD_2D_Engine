#include "View/window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QImage image(1200, 1600, QImage::Format_RGB32);
    image.fill(Qt::white);

    Scene scene;
    auto drawer = std::make_unique<Drawer>(&scene);
    auto input_handler = std::make_unique<InputHandler>(&scene);

    Window window(std::move(drawer), std::move(input_handler));
    window.show();
    return app.exec();

}