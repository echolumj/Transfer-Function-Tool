#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include "GLwidget.h"
#include <QVulkanInstance>
#include "Vec.h"

class TFDesigner;
class DiagDataAnalyzer;
class VulkanWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QWidget *wrapper;
    TFDesigner *myTFDesigner;
    DiagDataAnalyzer * diagDataAnalyzer;

    QVulkanInstance inst;
    VulkanWindow *vulkanWindow;

    Vec2f xAxis;
    Vec2f yAxis;

private:
    void createWindows(void);

private slots:
    void on_actionTransferFunction_Editor_toggled(bool arg1);
    void on_action_Open_triggered();
    void on_actionData_Analyzer_toggled(bool arg1);
    void on_actionPlay_toggled(bool arg1);

    void showViewStateOfTFDesigner(bool);
    void showViewStateOfPropertyEditor(bool);
    void showViewStateOfDiagDataAnalyzer(bool);

public slots:
    void createVulkanWindow(void);

};

#endif // MAINWINDOW_H
