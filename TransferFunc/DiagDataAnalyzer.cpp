#include "DiagDataAnalyzer.h"
#include "ui_DiagDataAnalyzer.h"
#include "iohelper.h"

#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <vtkXMLMultiBlockDataReader.h>
#include <vtkMultiBlockDataSet.h>

#include <vtkPointData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkDataArray.h>

#include <vtkGradientFilter.h>
#include <vtkDataSetGradient.h>
#include <vtkDataSetAlgorithm.h>
#include <vtkDataSet.h>
#include <QCheckBox>

#include <algorithm>
#include <datainfo.h>


DiagDataAnalyzer::DiagDataAnalyzer(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DiagDataAnalyzer)

{
    ui->setupUi(this);
    ui->spinBoxCurrentTime->setKeyboardTracking(false);
    ui->pushButtonRead->setDisabled(true);

    connect(&varsSelectionButtonGroup, SIGNAL(buttonClicked(int)),this, SLOT(varsSelectionButtonGroup_buttonClicked(int)));
    ui->buttonLayout->setAlignment(Qt::AlignCenter);
}


DiagDataAnalyzer::~DiagDataAnalyzer()
{
    delete ui;
    //delete classifiedData;

}

//slot functions
void DiagDataAnalyzer::on_pushButtonInputFileName_clicked()
{
    ui->pushButtonRead->setDisabled(false);
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);

    QString fileName;
    if (dialog.exec())
        fileName = dialog.selectedFiles().at(0);

    ui->lineEditInputFileName->setText(fileName);
}

void DiagDataAnalyzer::on_pushButtonRead_clicked()
{
    ui->pushButtonRead->setDisabled(true);
    QString fileName = ui->lineEditInputFileName->text();
    if(fileName == "")
        return;

    globalData->calcNameOfFileSeries(fileName);

    ui->labelTotalTimeStep->setText(QString("of %1").arg(globalData->totalTimeStep));
    ui->spinBoxCurrentTime->setRange(0,globalData->totalTimeStep-1);
    ui->spinBoxCurrentTime->setValue(globalData->currentTimeStep);

    globalData->readTimeStep(globalData->currentTimeStep);
    selectedVarInOrigin.clear();

    updateDataInfoPad();

    emit createVolume();
}



void DiagDataAnalyzer::clearDataInfo()
{
    int num = ui->tableWidgetVarsInfo->rowCount();
    if (num==0)
        return;

    for (int i=0; i<num; ++i){

     varsSelectionButtonGroup.removeButton(static_cast<QAbstractButton*>(ui->tableWidgetVarsInfo->cellWidget(i,0)) );
     delete ui->tableWidgetVarsInfo->cellWidget(i,0);
    }

    ui->tableWidgetVarsInfo->clear();
    ui->tableWidgetVarsInfo->setRowCount(0);
    selectedVarInOrigin.clear();
}
/*
vtkDataArray* DiagDataAnalyzer::calcGradientArray(vtkUnstructuredGrid * data, QString arrayName)
{
    auto gradientFilter = vtkGradientFilter::New();
    gradientFilter->SetInputData(data);
    gradientFilter->SetInputScalars(vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayName.toLatin1().data());
    gradientFilter->ComputeGradientOn();
    gradientFilter->SetResultArrayName("gradient");
    gradientFilter->Update();

    auto dataArray = gradientFilter->GetOutput()->GetPointData()->GetArray("gradient");

    if(dataArray->GetNumberOfComponents() == 9)
    {
        for(int index = 0; index < dataArray->GetNumberOfTuples(); index++)
        {
            auto tuple = dataArray->GetTuple(index);
            dataArray->SetComponent(index,0,sqrt(tuple[0] * tuple[0] + tuple[1] * tuple[1] + tuple[2] * tuple[2]));
            dataArray->SetComponent(index,1,sqrt(tuple[3] * tuple[3] + tuple[4] * tuple[4] + tuple[5] * tuple[5]));
            dataArray->SetComponent(index,2,sqrt(tuple[6] * tuple[6] + tuple[7] * tuple[7] + tuple[8] * tuple[8]));
        }
    }
    else
    {
        for(int index = 0; index < dataArray->GetNumberOfTuples(); index++)
        {
            auto tuple = dataArray->GetTuple(index);
            dataArray->SetComponent(index,0,sqrt(tuple[0] * tuple[0] + tuple[1] * tuple[1] + tuple[2] * tuple[2]));
        }
    }
    //return gradientFilter->GetOutput()->GetPointData()->GetArray("gradient");
    return dataArray;
}*/

/*
vtkDataArray* DiagDataAnalyzer::calcGradientArray(vtkUnstructuredGrid * data, int id)
{
    if(!data) return nullptr;
    return calcGradientArray(data, data->GetPointData()->GetArrayName(id));
}*/

void DiagDataAnalyzer::updateDataInfoPad()
{
    clearDataInfo();

    vtkRectilinearGrid * data = static_cast<vtkRectilinearGrid *>(globalData->data);
    Vec3i dim = globalData->getDim();

    int numPnts = static_cast<int>(data->GetNumberOfPoints());

    ui->labelExtentInfo->setText(QString("Extents:%1, %2, %3. NumPoints:%4").arg(dim.x).arg(dim.y).arg(dim.z).arg(numPnts));

    vtkPointData * 	pointdata = data->GetPointData ();

    int numVars = pointdata->GetNumberOfArrays();

    //init var : temporal gradient data
    auto scalarId = 0;
    ui->tableWidgetVarsInfo->setRowCount(numVars);
    for (int i=0; i< numVars; ++i){
        vtkDataArray *curVar= pointdata->GetArray(i);
        //int componentNum = curVar->GetNumberOfComponents();
        if(!strcmp(curVar->GetName(),temporalName.c_str()) && scalarId == 0)
            scalarId = i;

        QCheckBox * rb = new QCheckBox(ui->tableWidgetVarsInfo);
        ui->tableWidgetVarsInfo->setCellWidget(i,0,rb);
        varsSelectionButtonGroup.addButton(rb,i);

        QString name = curVar->GetName();
        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        ui->tableWidgetVarsInfo->setItem(i,1,nameItem);

        QString type = curVar->GetClassName();
        QTableWidgetItem *typeItem = new QTableWidgetItem(type);
        ui->tableWidgetVarsInfo->setItem(i,2,typeItem);

        QTableWidgetItem *rangeItem = nullptr;

        //vector-three
       auto range = curVar->GetRange();
       QString rangeStr = QString("[%1,%2]").arg(range[0],8,'g',4).arg(range[1],8,'g',4);
       //when array has multiple components, getRange will return the range of first component.
       //just show the first component range
       /*
       for(int subIndex = 1; subIndex < componentNum; subIndex++)
       {
           curVar->CopyComponent(0, curVar, subIndex);
           range = curVar->GetRange();
           rangeStr += QString(" , [%1,%2]").arg(range[0],8,'g',4).arg(range[1],8,'g',4);
       }
       */
        rangeItem = new QTableWidgetItem(rangeStr);

        ui->tableWidgetVarsInfo->setItem(i,3,rangeItem);
    }
    //if (selectedVarInOrigin.empty())
    //      selectedVarInOrigin.insert(0);
    selectedVarInOrigin.insert(scalarId);
    for(auto &it:selectedVarInOrigin)
        varsSelectionButtonGroup.button(it)->setChecked(true);

    //auto gradient = calcGradientArray(data,0);
    auto gradient = pointdata->GetArray(temporalName.c_str());
    auto scalar = pointdata->GetArray(scalarName.c_str());

    emit globalData->setTFSignal2D(scalar, gradient);
}

void DiagDataAnalyzer::on_pushButtonOutputFileName_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);

    QString fileName;
    if (dialog.exec())
        fileName = dialog.selectedFiles().at(0);

    //ui->lineEditOutputFileName->setText(fileName);
    //structureData->outputDir = fileName;
}

void DiagDataAnalyzer::on_pushButtonLoop_clicked(bool checked)
{
    if (loopFlag!=checked)
        loopFlag = checked;
}


void DiagDataAnalyzer::on_pushButtonRun_clicked()
{
}


//for now, just one parameter can be selected
void DiagDataAnalyzer::varsSelectionButtonGroup_buttonClicked(int id)
{
  selectedVarInOrigin.clear();
  selectedVarInOrigin.insert(id);

  vtkRectilinearGrid * data = static_cast<vtkRectilinearGrid *>(globalData->data);
  vtkPointData * pointdata = data->GetPointData();
/*
  auto gradientFilter = vtkSmartPointer<vtkDataSetGradient>::New();
  gradientFilter->SetInputData(data);
  gradientFilter->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS, pointdata->GetArrayName(id));
  gradientFilter->SetResultArrayName("gradient");
  gradientFilter->Update();
*/

  //always set scalar data(pressure) as x-axis
  auto arrayV = pointdata->GetArray(id);
  emit globalData->setTFSignal2D(pointdata->GetArray(scalarName.c_str()), arrayV);
}

void DiagDataAnalyzer::on_spinBoxCurrentTime_valueChanged(int arg1)
{
    globalData->readTimeStep(arg1);
    globalData->currentTimeStep = arg1;
    selectedVarInOrigin.clear();
    updateDataInfoPad();
    emit renewVolumeData();
}

void DiagDataAnalyzer::on_triangleButton_clicked()
{
    tfp::TransFuncTriangle* triangle = new tfp::TransFuncTriangle();
    emit addPrimitives(triangle);
    delete  triangle;
}

void DiagDataAnalyzer::on_quadButton_clicked()
{
    tfp::TransFuncQuad* quad = new tfp::TransFuncQuad();
    emit addPrimitives(quad);
    delete quad;
}

void DiagDataAnalyzer::on_bananaButton_clicked()
{
    tfp::TransFuncBanana* banana = new tfp::TransFuncBanana();
    emit addPrimitives(banana);
    delete banana;
}
