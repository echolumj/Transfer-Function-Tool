#include "TFDesigner.h"
#include "ui_TFDesigner.h"

#include <algorithm>
#include <numeric>
#include "datainfo.h"

#include <thread>
#include <QMutex>
/***********************************************************************
 *controlPoint:
 ******range:[0.0, 1.0]
 ******benchmark:chart()->plotArea()
 *offset:
 ******range:[0.0, 1.0]
 ******benchmark:chart()->plotArea()
************************************************************************/

QMutex locker;

//-----------------------------ChartView-----------------------------------------
ChartView::ChartView():
    pressFlag(false),moveAllFlag(false),pressPos(0)

{
}

void ChartView::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);

    tfImg = tfImg.scaled(static_cast<int>(TF_SIZE_WIDTH),static_cast<int>(TF_SIZE_HEIGHT));
    tfImg.fill(QColor(255,255,255,10));
    repaint();
    //saveImg("");
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    tfImg.fill(QColor(255,255,255,10));
    QPointF topLeft = this->chart()->plotArea().topLeft();
    if(event->button() == Qt::LeftButton) //change the position of controlPoints
    {
        QPointF choosePos = event->pos();
        size_t index = 0;

        for(std::vector<QPointF>::iterator it = controlPoints.begin(); it != controlPoints.end(); it++,index++)
        {
            if(std::abs(it->x() * this->chart()->plotArea().width()+ topLeft.x() - choosePos.x()) < 5
                    && std::abs(it->y() * this->chart()->plotArea().height() + topLeft.y() - choosePos.y()) < 5)
            {
                pressFlag = true;
                pressPos = index;
                break;
            }
        }
    }
    else if(event->button() == Qt::RightButton)//change the position of polygons
    {
        moveAllFlag = isInPolygon(event->pos());
        if(moveAllFlag)
        {
            for(size_t index = 0; index < offset.size(); index++)
            {
                offset[index] = QPointF(controlPoints[index].x(), controlPoints[index].y())
                        - QPointF((event->pos().x()-topLeft.x()) * 1.0/this->chart()->plotArea().width(),
                        (event->pos().y()-topLeft.y())* 1.0/this->chart()->plotArea().height());
            }
        }
    }
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF topLeft = this->chart()->plotArea().topLeft();
    if(pressFlag)
    {
       QPointF currentPos = event->pos();
       controlPoints[pressPos] = QPointF((currentPos.x()-topLeft.x()) * 1.0/ this->chart()->plotArea().width(), (currentPos.y()-topLeft.y()) * 1.0/ this->chart()->plotArea().height());
       update();
    }
    else if(moveAllFlag)
    {
        //QPointF offset =  event->pos() - mousePos;
        for(size_t index = 0; index < offset.size(); index++)
        {
            controlPoints[index] = QPointF((event->pos().x()-topLeft.x())* 1.0 / this->chart()->plotArea().width(), (event->pos().y()-topLeft.y())* 1.0 / this->chart()->plotArea().height()) + offset[index];
        }
        update();
    }
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    pressFlag = false;
    moveAllFlag = false;

    //saveImg("");
}

void ChartView::keyPressEvent(QKeyEvent *e)
{
    //const float amount = e->modifiers().testFlag(Qt::ShiftModifier) ? 1.0f : 0.1f;
    switch (e->key()) {
    case Qt::Key_Space:
        //emit transImage(tfImg);
        saveImg("");
        break;
    default:
        break;
    }
}

void ChartView::paintEvent(QPaintEvent *event)
{
    QChartView::paintEvent(event);//★

    if(controlPoints.size() > 0)
    {
        //primitives
        QPainter painter(this->viewport());
        painter.setRenderHint( QPainter::Antialiasing, true);
        painter.setBrush(QBrush(QColor(0,176,13)));

        QPen pen;
        pen.setColor(QColor(0, 0, 200, 50));
        painter.setPen(pen);

        vector<QPointF> midPoints;
        midPoints.resize(controlPoints.size());
        for(size_t i = 0; i < controlPoints.size(); i++)
        {
            midPoints[i] = QPointF(controlPoints[i].x() * this->chart()->plotArea(). width(), controlPoints[i].y()*
                                   this->chart()->plotArea().height()) +this->chart()->plotArea().topLeft();
        }
       painter.drawPolygon(&midPoints[0],static_cast<int>(midPoints.size()));
    }
}

void ChartView::drawPrimitive(tfp::TFPrimitive *primitive)
{
    size_t size = primitive->getNumControlPoints();

    if(size == 0)
    {
        std::runtime_error("the size of points is zero");
    }

    controlPoints.resize(size);
    offset.resize(size);
    color.resize(size);
    for(size_t index = 0; index < size; index++)
    {
        controlPoints[index] =QPointF(primitive->getControlPoint(index).position_.x, primitive->getControlPoint(index).position_.y);
        color[index] = primitive->getControlPoint(index).color_;
    }
    //saveImg("C:\\Users\\Echolmj\\Desktop\\1.png");
    update();
}

bool ChartView::isInPolygon(QPointF p)
{
    int crossNum = 0;
    size_t size = controlPoints.size();
    QPointF topLeft = this->chart()->plotArea().topLeft();
    for(size_t i = 0; i < size; i++)
    {
        QPointF p1 = QPointF(controlPoints[i].x()*this->chart()->plotArea().width()+topLeft.x(),
                             controlPoints[i].y()*this->chart()->plotArea().height()+topLeft.y());
        QPointF p2 = QPointF(controlPoints[(i+1) % size].x()*this->chart()->plotArea().width()+topLeft.x(),
                             controlPoints[(i+1) % size].y()*this->chart()->plotArea().height()+topLeft.y());

        if(std::fabs(p1.y() - p2.y()) < 0.0001)
            continue;
        if(p.y() < std::min(p1.y(), p2.y()))
            continue;
        if(p.y() >= std::max(p1.y(), p2.y()))
            continue;
         double x = (p.y() - p1.y())*(p2.x() - p1.x()) / (p2.y() - p1.y()) + p1.x();
         if (x > p.x())
              crossNum++; //只统计单边交点
    }
    return (crossNum % 2 == 1);
}


void ChartView::saveImg(QString filePath)
{
    //primitives
    //tfimg init -- one primitive every time
    tfImg = QImage(QSize(static_cast<int>(TF_SIZE_WIDTH), static_cast<int>(TF_SIZE_HEIGHT)), QImage::Format_RGBA8888);
    tfImg.fill(QColor(255,255,255,10));

    QPainter tfPainter(&tfImg);
    tfPainter.setRenderHint( QPainter::Antialiasing, true );
    tfPainter.setBrush(QBrush(QColor(0,255,0,200)));
    tfPainter.setPen(QColor(0,255,0,200));
    if(controlPoints.size() > 0)
    {
        std::vector<QPointF> midV;
        midV.resize(controlPoints.size());
        for(size_t i = 0; i < midV.size(); i++)
            midV[i] = QPointF(controlPoints[i].x() * 1.0f * TF_SIZE_WIDTH, controlPoints[i].y() * 1.0f * TF_SIZE_HEIGHT);
        tfPainter.drawPolygon(&midV[0],static_cast<int>(midV.size()));
    }
    tfImg.save(filePath + "tf.png");
    emit transImage(tfImg);
}

//-----------------------------TFDesigner-----------------------------------------
TFDesigner::TFDesigner(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::TFDesigner)
{
    ui->setupUi(this);

    ui->gridLayout->setAlignment(this, Qt::AlignTop);

    chartsInit(0);
    chartsInit(1);
    chartsInit(2);


    ui->gridLayout->setRowStretch(0,5);
    ui->gridLayout->setRowStretch(1,1);
    ui->gridLayout->setRowStretch(2,1);

    ui->gridLayout->setColumnStretch(0,1);
}

TFDesigner::~TFDesigner()
{
    clearTF();
    delete ui;
}

void TFDesigner::chartsInit(int id)
{
    this->myChart[id] = new QChart();
    this->chartWidget[id] = new ChartView();
    //this->chartWidget[id]->setParent(this);

    this->myScatterSeries[id] = new QScatterSeries();
    this->myScatterSeries[id]->setUseOpenGL(true);            //启用OpenGL，否则可能会很卡顿
    //this->myScatterSeries[id]->setMarkerShape(QScatterSeries::MarkerShapeCircle);//设置散点样式
    this->myScatterSeries[id]->setColor(QColor(0,0,255));     //设置散点颜色

    this->myChart[id]->addSeries(this->myScatterSeries[id]);
    this->myChart[id]->createDefaultAxes();
    this->myChart[id]->legend()->hide();                                    //隐藏图例
    this->myChart[id]->setTitle(QString("component_%1").arg(id+1));

    this->chartWidget[id]->setChart(this->myChart[id]);
    this->chartWidget[id]->setRubberBand(QChartView::RectangleRubberBand);   //拉伸效果
    //this->chartWidget[id]->chart()->setAnimationOptions(QChart::AllAnimations);

    //this->chartWidget[id]->setVisible(false);
    this->chartWidget[id]->setVisible(false);
    ui->gridLayout->addWidget(this->chartWidget[id],id, 0);
}

TransferFunction* TFDesigner::getTFDefault(void)
{
    if(tfmap.empty())
        return nullptr;

    return tfmap.begin()->second->tf;
}

TransferFunction* TFDesigner::getTF(QString name)
{
    if(tfmap[name] != nullptr)
        return tfmap[name]->tf;
    else
        return nullptr;

}

void TFDesigner::addOneTF(vtkDataArray* dataArray, QString name)
{
    TFComponent *tfcomp = new TFComponent(name);
    //create tf
    tfcomp->tf = new TransferFunction();
    tfcomp->tf->setDataRange(getMin(dataArray), getMax(dataArray));

    tfmap[name] = tfcomp;
}

void TFDesigner::addOneTF(vtkDataArray* dataArray, vtkDataArray* gradient, QString name)
{
    TFComponent *tfcomp = new TFComponent(name);
    //create tf
    tfcomp->tf = new TransferFunction();
    tfcomp->tf->setDataRange(getMin(dataArray), getMax(dataArray));

    tfmap[name] = tfcomp;
}

void TFDesigner::toggleOneTF(QString name, bool visible)
{
    if(  tfmap[name]->visibleFlag != visible )
    {
        tfmap[name]->visibleFlag = visible;
    }

}

void TFDesigner::delOneTF(QString name)
{
    if (tfmap.find(name)!= tfmap.end() )
        tfmap.erase ( tfmap.find(name));
}

void TFDesigner::deactiveOneTF(QString name)
{
    if (tfmap.find(name)!= tfmap.end() )
        toggleOneTF(name, false);
}

void TFDesigner::deactiveALLTF()
{

    for(auto &it:tfmap){
        toggleOneTF(it.first, false);
    }
}

void TFDesigner::activeOneTF(vtkDataArray *dataArray, QString name)
{

    //if (tfmap.find(name)!= tfmap.end() )
    //   toggleOneTF(name, true);
    //else
    //{
        clearTF();
        QLayoutItem *item;
        while((item = ui->gridLayout->takeAt(0)) != nullptr){
          /*  if(item->widget()){
                delete item->widget();
                //item->widget()->deleteLater();
            }*/
            ui->gridLayout->removeItem(item);
        }
        addOneTF(dataArray, name);
   // }
//    clearTF();
//    addOneTF(dataArray, name);
}

void TFDesigner::threadRead(QList<QPointF> &listPoint, vtkDataArray *dataArray, vtkDataArray *gradient, int start, int end)
{
    QPointF pointf;
    for(int i = start; i <= end; i=i+THREAD_NUM)
    {
        pointf.setX(dataArray->GetTuple(i)[0]);
        pointf.setY(gradient->GetTuple(i)[0]);
        locker.lock();
        listPoint.append(pointf);
        locker.unlock();
    }
}

void TFDesigner::activeOneTF(vtkDataArray *dataArray, vtkDataArray* gradient)
{
    QLayoutItem *item;

    uint8_t midIndex = 0;
    while((item = ui->gridLayout->itemAtPosition(midIndex,0)) != nullptr && midIndex < 3){
        if(item->widget()){
            item->widget()->setVisible(false);
        }
        midIndex++;
    }

    //gradient
    for(int subIndex = 0; subIndex < dataArray->GetNumberOfComponents(); subIndex++)
    {
        if(subIndex > 0)
        {
            dataArray->CopyComponent(0, dataArray, subIndex);
            gradient->CopyComponent(0, gradient, subIndex);
        }

        auto dataRange = dataArray->GetRange();
        auto gradientRange = gradient->GetRange();

        QList<QPointF> listPoint;
        //clear()+append = replace
        //set title name of axis
        this->myChart[subIndex]->axes(Qt::Horizontal).first()->setTitleText(dataArray->GetName());
        this->myChart[subIndex]->axes(Qt::Vertical).first()->setTitleText(gradient->GetName());

        //deal with situation with datarange[0] == datarange[1]
        if(dataRange[1] - dataRange[0] < 0.00001)
        {
            this->myChart[subIndex]->axes(Qt::Horizontal).first()->setRange(dataRange[0]-0.1, dataRange[1]+0.1); //设置水平坐标范围
            this->myChart[subIndex]->axes(Qt::Vertical).first()->setRange(dataRange[0]-0.1, dataRange[1]+0.1);       //设置垂直坐标范围

            listPoint.append(QPointF(dataRange[0], dataRange[1]));
            this->myScatterSeries[subIndex]->replace(listPoint);
            this->chartWidget[subIndex]->setVisible(true);
            continue;
        }

        //set data range of axis
        this->myChart[subIndex]->axes(Qt::Horizontal).first()->setRange(dataRange[0], dataRange[1]); //设置水平坐标范围
        this->myChart[subIndex]->axes(Qt::Vertical).first()->setRange(gradientRange[0], gradientRange[1]);       //设置垂直坐标范围

        QPointF pointf;
        for(int i = 0; i < dataArray->GetNumberOfTuples(); i=i+80)
        {
            pointf.setX(dataArray->GetTuple(i)[0]);
            pointf.setY(gradient->GetTuple(i)[0]);
            listPoint.append(pointf);
        }

         this->myScatterSeries[subIndex]->replace(listPoint);//replace(listPoint); //更新散点
         this->chartWidget[subIndex]->setVisible(true);
    }

    //get the size of chart plot area
    //QSizeF plotAreaSize = this->chartWidget[0]->chart()->plotArea().size();
    //plotAreaSize.width()), static_cast<int>(plotAreaSize.height()))
    chartWidget[0]->tfImg = QImage(QSize(static_cast<int>(TF_SIZE_WIDTH), static_cast<int>(TF_SIZE_HEIGHT)), QImage::Format_RGBA8888);
    chartWidget[1]->tfImg = QImage(QSize(static_cast<int>(TF_SIZE_WIDTH), static_cast<int>(TF_SIZE_HEIGHT)), QImage::Format_RGBA8888);
    chartWidget[2]->tfImg = QImage(QSize(static_cast<int>(TF_SIZE_WIDTH), static_cast<int>(TF_SIZE_HEIGHT)), QImage::Format_RGBA8888);

    chartWidget[0]->tfImg.fill(QColor(255,255,255,10));
    chartWidget[1]->tfImg.fill(QColor(255,255,255,10));
    chartWidget[2]->tfImg.fill(QColor(255,255,255,10));
}


int TFDesigner::getVisibleTFNum()
{
    //return std::accumulate(tfmap.begin(),tfmap.end(),0, [](TFComponent* & it){ return it->visibleFlag?1:0;});

    int count = 0;
     map<QString, TFComponent*>::iterator it;
     for (it = tfmap.begin(); it!=tfmap.end(); ++it){
         if (it->second->visibleFlag)
             ++count;
     }
     return count;
}

void TFDesigner::clearTF()
{

    if ( tfmap.empty())
        return;

    //for_each(tfmap.begin(),tfmap.end(),[](TFComponent* it){delete it;});
    tfmap.clear();
}

