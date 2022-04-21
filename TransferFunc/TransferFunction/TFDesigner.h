#ifndef TFDesigner_H
#define TFDesigner_H

#include <QDockWidget>
#include "TransferFunction.h"

#include <vtkDataArray.h>

#include <vector>
#include <map>

#include <QtCharts>
#include<QtCharts/QScatterSeries>

#include "TFPrimitive.h"
//#include <QList>

QT_CHARTS_USE_NAMESPACE
using namespace std;

#define TF_SIZE_WIDTH 400
#define TF_SIZE_HEIGHT 400

class QCustomPlot;
class QCPGraph;

namespace Ui {
class TFDesigner;
}

class ChartView : public QChartView
{
    Q_OBJECT
public slots:
    void drawPrimitive(tfp::TFPrimitive* primitive);
public:
    QPainterPath painterPath;
    QImage tfImg;

    void saveImg(QString filePath);
    ChartView();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *);

private:
    bool pressFlag;
    bool moveAllFlag;
    size_t pressPos;

   std::vector<QPointF> controlPoints;
   std::vector<QPointF> offset;
   std::vector<tfp::col4> color;

   bool isInPolygon(QPointF p);

signals:
   void transImage(QImage);
};

class TFComponent {
public:
    TFComponent(QString name)
    {
        this->name=name;
        tf = nullptr;
        visibleFlag = true;
    }
    ~TFComponent()
    {
        delete tf;
    }
    QString name;  // name rules: <dataSet Name>_<vars' id>
    TransferFunction* tf;
    bool visibleFlag;
};


class TFDesigner : public QDockWidget
{
    Q_OBJECT

public:
    ChartView *chartWidget[3];
    QCustomPlot *pCustomPlot;
    QCPGraph * curGraph;

public:
    explicit TFDesigner(QWidget *parent = nullptr);
    ~TFDesigner();

    map<QString, TFComponent*>  getTFList(){return tfmap;}
    TransferFunction* getTFDefault(void);
    TransferFunction* getTF(QString name);
    int getVisibleTFNum();
    void clearTF();

private:
    void chartsInit(int id);
    void threadRead(QList<QPointF>&, vtkDataArray*, vtkDataArray*, int start, int end);

private:
    Ui::TFDesigner *ui;
    bool updateAutoFlag;
    map<QString, TFComponent*> tfmap; //save tf texture

    QChart *myChart[3];
    QScatterSeries *myScatterSeries[3]; //散点类型

public slots:
  void addOneTF(vtkDataArray* dataArray, QString name);
  void addOneTF(vtkDataArray *dataArray, vtkDataArray*, QString name);

   void activeOneTF(vtkDataArray *dataArray, QString name);
   void activeOneTF(vtkDataArray *dataArray, vtkDataArray*);

  void delOneTF(QString name);
  void deactiveOneTF(QString name);
  void toggleOneTF(QString name, bool visible);
  void deactiveALLTF();

signals:
    void updateVRView();
    void TFModified();

};

#endif // TFDesigner_H
