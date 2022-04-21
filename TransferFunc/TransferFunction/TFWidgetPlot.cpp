#include "TFWidgetPlot.h"
#include "colorbar.h"
#include <qevent.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_wheel.h>
#include <qwt_plot_directpainter.h>
#include <stdlib.h>
//#include <QBrush>


TFWidgetPlot::TFWidgetPlot( QWidget *parent ):
    QwtPlot( parent ),
    histFillcolor(0.4,0,0,0.1),
    myTF(NULL),histCurve(NULL),opaqueTFCurve(NULL)
{
    //setTitle( "Interactive Plot" );

    //setCanvasColor( Qt::darkCyan );

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajorPen( Qt::white, 0, Qt::DotLine );
    grid->attach( this );



    // ------------------------------------
    // We add a color bar to the left axis
    // ------------------------------------

    QwtScaleWidget *scaleWidget = axisWidget( xBottom );
    scaleWidget->setMargin( 30 ); // area for the color bar
    d_colorBar = new ColorBar( Qt::Horizontal, scaleWidget );
    d_colorBar->setPlot(this);
    d_colorBar->setRange( Qt::red, Qt::darkBlue );
    //d_colorBar->setFocusPolicy( Qt::TabFocus );
    connect(this,SIGNAL(TFModified()),d_colorBar,SLOT(update()));



    // we need the resize events, to lay out the color bar
    scaleWidget->installEventFilter( this );

     // we need the resize events, to lay out the wheel
    canvas()->installEventFilter( this );


    setAutoReplot(true);

}
TFWidgetPlot::~TFWidgetPlot()
{
    cleanPlot();
}

void TFWidgetPlot::setCanvasColor( const QColor &c )
{
    setCanvasBackground( c );
    replot();
}

void TFWidgetPlot::flush()
{
    /*
       Enable QwtPlotCanvas::ImmediatePaint, so that the canvas has been
       updated before we paint the cursor on it.
     */
    QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>(canvas());

    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, true );
    replot();
    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, false );

}

void TFWidgetPlot::scrollLeftAxis( double value )
{
//    setAxisScale( yLeft, value, value + 100.0 );
//    replot();
}

bool TFWidgetPlot::eventFilter( QObject *object, QEvent *e )
{
    if ( e->type() == QEvent::Resize )
    {
        const QSize size = static_cast<QResizeEvent *>( e )->size();
        if ( object == axisWidget( xBottom ) )
        {
            const QwtScaleWidget *scaleWidget = axisWidget( xBottom );

            const int margin = 2;

            // adjust the color bar to the scale backbone
//            const int x = size.width() - scaleWidget->margin() + margin;
//            const int w = scaleWidget->margin() - 2 * margin;
//            const int y = scaleWidget->startBorderDist();
//            const int h = size.height() -
//                scaleWidget->startBorderDist() - scaleWidget->endBorderDist();
            const int x = scaleWidget->startBorderDist();
            const int w = size.width()-scaleWidget->startBorderDist() - scaleWidget->endBorderDist();
            const int y = 0;//scaleWidget->margin() + margin;
            const int h = size.height() -scaleWidget->margin()-margin;
            d_colorBar->setGeometry( x, y, w, h );
        }
//        if ( object == canvas() )
//        {
//            const int w = 16;
//            const int h = 50;
//            const int margin = 2;

//            const QRect cr = canvas()->contentsRect();
//         }
    }

    return QwtPlot::eventFilter( object, e );
}

void TFWidgetPlot::setVar(vtkDataArray *var)
{
    if (this->var) {
         cleanPlot();
    }
    this->var = var;  //current we just ref to the pointer. If necessary in future, do a deepcopy here.
    if(var){
        setAxisScale( QwtPlot::xBottom, getMin(var), getMax(var) );
        setAxisScale( QwtPlot::yLeft, 0.0, 1.0 );
        setAxisTitle(QwtPlot::yLeft,var->GetName());
        createHistogramPlot();
        replot();
    }
}

void TFWidgetPlot::setVar(vtkDataArray* var, vtkDataArray* gradient)
{
    this->gradient = gradient;
    setVar(var);
}

void TFWidgetPlot::setTF(TransferFunction *tf)
{
    myTF = tf;
    createOpaqueTFPlot();
    replot();
}

QwtPlotCurve* TFWidgetPlot::getOpaqueTFCurve()
{
    return opaqueTFCurve;
}

void TFWidgetPlot::insertCurve( int axis, double base )
{
    Qt::Orientation o;
    if ( axis == yLeft || axis == yRight )
        o = Qt::Horizontal;
    else
        o = Qt::Vertical;

    QRgb rgb = static_cast<QRgb>( rand() );
    insertCurve( o, QColor( rgb ), base );
    replot();
}

void TFWidgetPlot::insertCurve( Qt::Orientation o,
    const QColor &c, double base )
{
    QwtPlotCurve *curve = new QwtPlotCurve();

    curve->setPen( c );
    curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
        Qt::gray, c, QSize( 8, 8 ) ) );

    double x[10];
    double y[sizeof( x ) / sizeof( x[0] )];

    for ( uint i = 0; i < sizeof( x ) / sizeof( x[0] ); i++ )
    {
        double v = 5.0 + i * 10.0;
        if ( o == Qt::Horizontal )
        {
            x[i] = v;
            y[i] = base;
        }
        else
        {
            x[i] = base;
            y[i] = v;
        }
    }

    curve->setSamples( x, y, sizeof( x ) / sizeof( x[0] ) );
    curve->attach( this );
}

void TFWidgetPlot::createHistogramPlot()
{
    if(!var || !gradient)
        return;

    if (histCurve){
        histCurve->detach();
        delete histCurve;
        histCurve = nullptr;
    }
    histCurve = new QwtPlotCurve();

    histCurve->setPen( QColor::fromRgbF(histFillcolor.x,histFillcolor.y,histFillcolor.z,histFillcolor.w));
//    curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
//        Qt::gray, QColor(static_cast<QRgb>( rand() )), QSize( 8, 8 ) ) );

    updateHistogramPlot();
    histCurve->attach( this );
    //const QBrush brush(QColor::fromRgbF(histFillcolor.x,histFillcolor.y,histFillcolor.z,histFillcolor.w));
    //histCurve->setBrush(brush);

}
void TFWidgetPlot::updateHistogramPlot()
{
    //calculate the graident array

    //1d
    //vector<int> hist = calcHist(var);
    //2d
    vector<float> hist = calcGradientHist(var, gradient);
    double *x=new double[hist.size()];
    double *y=new double[hist.size()];
    double min = getMin(var);
    double max = getMax(var);

    double step=(max-min)/(hist.size()-1);

    double c_max = *std::max_element (hist.begin(),hist.end());
    //  highest point of histogram curve is map to 0.75 in y-axiss
    for ( uint i = 0; i < hist.size(); i++ ){
        x[i] = min + i*step;
        y[i] = static_cast<double>(hist[i])/c_max*0.5+0.5;//modified by lmj 11_10
    }

    histCurve->setSamples(x, y, hist.size());
    delete []x;
    delete []y;

}


void TFWidgetPlot::cleanPlot()
{
    detachItems(QwtPlotItem::Rtti_PlotItem, true);
    histCurve = nullptr;
    opaqueTFCurve = nullptr;
}

void TFWidgetPlot::createOpaqueTFPlot()
{
    if (!myTF)
        return;
    if (opaqueTFCurve){
        opaqueTFCurve->detach();
        delete opaqueTFCurve;
        opaqueTFCurve = nullptr;
    }

    opaqueTFCurve = new QwtPlotCurve();

    opaqueTFCurve->setPen( QColor::fromRgbF(0.9,0.1,0.1, 1 ));
    opaqueTFCurve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
         Qt::gray, QColor(100, 100, 100), QSize( 8, 8 ) ) ); //lmj★


    opaqueTFCurve->attach( this );
    updateOpaqueTFPlot();

}

void TFWidgetPlot::updateOpaqueTFPlot()
{
    list<pair<float,Vec4f>>  &keyPointsTF = myTF->keyPointsTF;
    int size = static_cast<int>(keyPointsTF.size()) ;
    double *x=new double[size];
    double *y=new double[size];

    list<pair<float,Vec4f>>::iterator it;

    int i=0;
    for ( it=keyPointsTF.begin(); it!=keyPointsTF.end(); ++it,++i){
        x[i] = it->first;
        y[i] = it->second.w;
    }

    opaqueTFCurve->setSamples(x, y, size);
    delete []x;
    delete []y;

    showOpaqueTFPlotColor();

}

void TFWidgetPlot::drawItems(QPainter *p, const QRectF &r, const QwtScaleMap maps[]) const
{

    QwtPlot::drawItems(p,r,maps);
    //showOpaqueTFPlotColor();
}

void TFWidgetPlot::drawCanvas(QPainter * p)
{
    QwtPlot::drawCanvas(p);

    //showOpaqueTFPlotColor();
}

void TFWidgetPlot::showOpaqueTFPlotColor() const {
    if ( !opaqueTFCurve )
        return;

    QwtSymbol *symbol = const_cast<QwtSymbol *>( opaqueTFCurve->symbol() );

    const QBrush brush = symbol->brush();

    QwtPlotDirectPainter directPainter;
    list<pair<float,Vec4f>>::iterator it = myTF->keyPointsTF.begin();
    for(int i=0; i<static_cast<int>(opaqueTFCurve->dataSize()) ; ++i,++it){
        Vec4f c = it->second;
        symbol->setBrush( QColor::fromRgbF(c.x,c.y,c.z) );
        directPainter.drawSeries( opaqueTFCurve, i, i );
    }

    symbol->setBrush( brush ); // reset brush
}
