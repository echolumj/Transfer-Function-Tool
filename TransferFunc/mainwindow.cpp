#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "TransferFunction/TFDesigner.h"
#include "DiagDataAnalyzer.h"
#include "shared/trianglerenderer.h"

#include <QString>
#include <QFile>
#include <QFileDialog>
#include <VulkanWindow.h>

MainWindow::MainWindow()
    :ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Volume Ray-casting");
    globalData = new StructureDataInfo;
    vulkanWindow = nullptr;

    createWindows();
}

MainWindow::~MainWindow()
{
    delete  globalData;
    if(vulkanWindow)
        delete vulkanWindow;
    delete myTFDesigner;
    delete diagDataAnalyzer;
    delete wrapper;
    delete ui;
}

void MainWindow::createWindows()
{
    myTFDesigner = new TFDesigner(this);
    addDockWidget(Qt::RightDockWidgetArea, myTFDesigner);

    diagDataAnalyzer = new DiagDataAnalyzer(this);
    addDockWidget(Qt::LeftDockWidgetArea,diagDataAnalyzer);

    //TF UI Changeed
    connect(globalData, SIGNAL(setTFSignal2D(vtkDataArray*, vtkDataArray*)),myTFDesigner,SLOT(activeOneTF(vtkDataArray*,vtkDataArray*)));
    connect(diagDataAnalyzer, SIGNAL(createVolume()), this, SLOT(createVulkanWindow()));
    connect(diagDataAnalyzer, SIGNAL(addPrimitives(tfp::TFPrimitive *)), myTFDesigner->chartWidget[0], SLOT(drawPrimitive(tfp::TFPrimitive *)));
}

void MainWindow::createVulkanWindow(void)
{
    if(vulkanWindow)
    {
        emit diagDataAnalyzer->renewVolumeData();
        return;
    }

    const bool dbg = qEnvironmentVariableIntValue("QT_VK_DEBUG");

    if (dbg) {
        QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

    #ifndef Q_OS_ANDROID
            inst.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
    #else
            inst.setLayers(QByteArrayList()
                           << "VK_LAYER_GOOGLE_threading"
                           << "VK_LAYER_LUNARG_parameter_validation"
                           << "VK_LAYER_LUNARG_object_tracker"
                           << "VK_LAYER_LUNARG_core_validation"
                           << "VK_LAYER_LUNARG_image"
                           << "VK_LAYER_LUNARG_swapchain"
                           << "VK_LAYER_GOOGLE_unique_objects");
    #endif
    }

    if (!inst.create())
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

    vulkanWindow = new VulkanWindow;
    vulkanWindow->setVulkanInstance(&inst);
    vulkanWindow->createRenderer();

    wrapper = QWidget::createWindowContainer(vulkanWindow,this);
    wrapper->setFocusPolicy(Qt::StrongFocus);
    wrapper->setFocus();
    this->setCentralWidget(wrapper);

    connect(diagDataAnalyzer, SIGNAL(renewVolumeData()), vulkanWindow, SLOT(volumeData_renew()));
    connect(myTFDesigner->chartWidget[0], SIGNAL(transImage(QImage)), vulkanWindow, SLOT(imageData_renew(QImage)));
}

void MainWindow::on_actionTransferFunction_Editor_toggled(bool arg1)
{
    if (arg1)
        myTFDesigner->show();
    else
        myTFDesigner->hide();
}

void MainWindow::on_actionData_Analyzer_toggled(bool arg1)
{
    if (arg1)
        diagDataAnalyzer->show();
    else
        diagDataAnalyzer->hide();
}


void MainWindow::on_action_Open_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);

    QString fileName;
    if (dialog.exec())
        fileName = dialog.selectedFiles().at(0);
}

//manually slots
void MainWindow::showViewStateOfTFDesigner(bool arge)
{
    ui->actionTransferFunction_Editor->setChecked(arge);
}

void MainWindow::showViewStateOfPropertyEditor(bool arge)
{
    ui->actionProperty_Editor->setChecked(arge);
}

void MainWindow::showViewStateOfDiagDataAnalyzer(bool arge)
{
    ui->actionData_Analyzer->setChecked(arge);
}

//not use now
void MainWindow::on_actionPlay_toggled(bool arg1)
{
    if (!arg1) {
        return;
    }

    while(1){
       QCoreApplication::processEvents();
       if (!ui->actionPlay->isChecked()) {
           return;
       }
    }
}
