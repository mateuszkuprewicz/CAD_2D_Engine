#include "window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QImage image(1200, 1600, QImage::Format_RGB32);
    image.fill(Qt::white);
    Window window(image);
    window.show();
    return app.exec();
}
