#include "mainwindow.h"
#include <QApplication>
QApplication *gapp;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gapp = &a;
    gmainwindow = new MainWindow;
    gmainwindow->setWindowTitle("Video Laboratory");
    gmainwindow->move(600,250);
    gmainwindow->init();
    gmainwindow->show();

    return a.exec();
}
