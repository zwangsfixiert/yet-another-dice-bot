#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Sigma Squared");
    QCoreApplication::setApplicationName("Yet Another Dice Bot");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
