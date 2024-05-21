#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    Py_Initialize();
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("LineSets");
    w.show();
    return a.exec();
}
