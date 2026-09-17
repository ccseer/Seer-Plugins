#include <QGuiApplication>
#include "histogramhelper.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    return runImageHistogram(app.arguments());
}
