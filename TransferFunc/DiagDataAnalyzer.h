#ifndef DiagDataAnalyzer_H
#define DiagDataAnalyzer_H

#include <vtkSmartPointer.h>
#include <QDockWidget>
#include <QButtonGroup>
#include <QString>
#include <QStringList>
#include <map>
#include <vector>
#include <list>
#include <set>
#include "TFPrimitive.h"
#include "datainfo.h"

//extern globalData

class vtkIntArray;
class vtkFloatArray;
class vtkDataArray;
class vtkUnstructuredGrid;
namespace Ui {
class DiagDataAnalyzer;
}


class DiagDataAnalyzer : public QDockWidget
{
    Q_OBJECT

public:
    explicit DiagDataAnalyzer(QWidget *parent = nullptr);
    ~DiagDataAnalyzer();


    void readTimeStep(int n);

   //void setDataInfo( StructureDataInfo *structureData);

signals:
    ///
    /// \brief addPrimitives
    /// \param index
    /// index = 0:triangle
    /// index = 1:quad
    /// index = 2:banana
    ///
    void addPrimitives(tfp::TFPrimitive* primitive);
    void createVolume();
    void renewVolumeData();

private slots:

    void on_pushButtonInputFileName_clicked();

    void on_pushButtonRead_clicked();

    void on_pushButtonOutputFileName_clicked();

    void on_pushButtonLoop_clicked(bool checked);

    void on_pushButtonRun_clicked();

    void varsSelectionButtonGroup_buttonClicked(int id);

    void on_spinBoxCurrentTime_valueChanged(int arg1);

    void on_triangleButton_clicked();

    void on_quadButton_clicked();

    void on_bananaButton_clicked();

protected:

    void setCurrentTimeStep(int t);

    void updateDataInfoPad();

    void clearDataInfo();

private: //vars
    Ui::DiagDataAnalyzer *ui;
    QButtonGroup varsSelectionButtonGroup;

    set<int> selectedVarInOrigin;

    //StructureDataInfo *structureData;  //set by mainwindow, but we can monify its value. don't create/delete it.

    bool loopFlag;
    bool classifyingFlag;

private:
    vtkDataArray* calcGradientArray(vtkUnstructuredGrid * data, QString arrayName);
    vtkDataArray* calcGradientArray(vtkUnstructuredGrid * data, int id);

};

#endif // DiagDataAnalyzer_H
