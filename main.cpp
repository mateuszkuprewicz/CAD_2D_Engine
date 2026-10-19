#include "window.h"
#include "View/drawer.h"
#include "Models/polygon.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QImage image(1200, 1600, QImage::Format_RGB32);
    Window window(image);
    Polygon polygon({QPoint(10,10), QPoint(100,10), QPoint(100, 100), QPoint(10,100)});
    Drawer drawer(image, {&polygon});
    drawer.Render();
    window.show();
    return app.exec();
}
