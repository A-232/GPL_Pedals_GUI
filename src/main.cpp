// GPL Smooth Pedals - Плавные педали для Grand Prix Legends
// Автор: [A-232]
// Компилятор: MinGW UCRT64

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GPL Smooth Pedals");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.show();

    return app.exec();
}