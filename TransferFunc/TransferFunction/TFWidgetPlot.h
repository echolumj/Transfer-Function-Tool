#ifndef TFWidgetPlot_H
#define TFWidgetPlot_H

#include "qwt_plot.h"
#include "TransferFunction.h"
#include "../datainfo.h"
#include "vtkDataArray.h"


class ColorBar;
class QwtWheel;
class QwtPlotCurve;

class TFWidgetPlot: public QwtPlot
{
    Q_OBJECT
public:
    TFWidgetPlot( QWidget *parent = nullptr );
    ~TFWidgetPlot();
    virtual bool eventFilter( QObject *, QEvent * );
    void setVar(vtkDataArray* var);
    void setVar(vtkDataArray* var, vtkDataArray* gradient);

    void setTF(TransferFunction * tf);
    virtual void drawItems (QPainter *, const QRectF &, const QwtScaleMap maps[axisCnt]) const;
    virtual void 	drawCanvas (QPainter *);
    QwtPlotCurve* getOpaqueTFCurve();
    TransferFunction* getTF() { return myTF; }

    void flush();
public Q_SLOTS:
    void setCanvasColor( const QColor & );
    void insertCurve( int axis, double base );
    void createHistogramPlot();
    void updateHistogramPlot();
    void createOpaqueTFPlot();
    void updateOpaqueTFPlot();
    void showOpaqueTFPlotColor() const;
    void cleanPlot();

private Q_SLOTS:
    void scrollLeftAxis( double );

signals:
    void TFModified();

private:
    void insertCurve( Qt::Orientation, const QColor &, double base );

    // do not own it. it belong to TFDesigner.
    TransferFunction * myTF;

    vtkDataArray* var = nullptr;
    vtkDataArray* gradient = nullptr;

    ColorBar *d_colorBar;
    QwtWheel *d_wheel;

    bool showHistogram;
    Vec4f histFillcolor;
    QwtPlotCurve *histCurve;
    QwtPlotCurve *opaqueTFCurve;

};




#endif









