
#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <VulkanWindow.h>

Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
