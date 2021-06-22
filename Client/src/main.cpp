#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile file(":/style/style.css");
    file.open(QFile::ReadOnly);

    QString styles(file.readAll());
    a.setStyleSheet(styles);

    MainWindow m;
    m.show();
    return a.exec();
}
