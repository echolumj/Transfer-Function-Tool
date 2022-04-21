#include <qapplication.h>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <QColorDialog>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include "TFInteraction.h"
#include "TFWidgetPlot.h"

TFInteraction::TFInteraction( TFWidgetPlot *plot ):
    QObject( plot ),
    d_selectedPoint( -1 )
{
    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>( plot->canvas() );
    canvas->installEventFilter( this );

    // We want the focus, but no focus rect. The
    // selected point will be highlighted instead.

    canvas->setFocusPolicy( Qt::StrongFocus );
#ifndef QT_NO_CURSOR
    canvas->setCursor( Qt::PointingHandCursor );
#endif
    canvas->setFocusIndicator( QwtPlotCanvas::ItemFocusIndicator );
    canvas->setFocus();

}

TFWidgetPlot *TFInteraction::plot()
{
    return qobject_cast<TFWidgetPlot *>( parent() );
}

const TFWidgetPlot *TFInteraction::plot() const
{
    return qobject_cast<const TFWidgetPlot *>( parent() );
}

//bool TFInteraction::event( QEvent *ev )
//{
//    if ( ev->type() == QEvent::User )
//    {
//        plot()->showOpaqueTFPlotColor();
//        return true;
//    }
//    return QObject::event( ev );
//}

bool TFInteraction::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == nullptr || object != plot()->canvas() )
        return false;

    switch( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
            select( mouseEvent->pos() );
            return true;
        }
        case QEvent::MouseMove:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
            move( mouseEvent->pos() );
            return true;
        }
        case QEvent::MouseButtonDblClick:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
            doubleClicked( mouseEvent->pos() );
            return true;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
            const int delta = 1;
            switch( keyEvent->key() )
            {
                // The following keys represent a direction, they are
                // organized on the keyboard.

                case Qt::Key_Up:
                {
                    moveBy( 0, delta );
                    break;
                }
                case Qt::Key_Left:
                {
                    moveBy( -delta, 0 );
                    break;
                }
                case Qt::Key_Right:
                {
                    moveBy( delta, 0 );
                    break;
                }
                case Qt::Key_Down:
                {
                    moveBy( 0, -delta );
                    break;
                }
                case Qt::Key_Delete:
                {
                    deleteSelected();
                    break;
                }
                default:
                    break;
            }
        }

        default:
            break;
    }
    plot()->showOpaqueTFPlotColor();
    return QObject::eventFilter( object, event );
}

void TFInteraction::deleteSelected()
{
    if (d_selectedPoint == -1)
        return;
    TransferFunction * TF = plot()->getTF();
    TF->removeKeypoint_byIndex(d_selectedPoint);

    plot()->updateOpaqueTFPlot();
    plot()->flush();
    emit plot()->TFModified();

}

// Select the point at a position. If there is no point
// deselect the selected point

void TFInteraction::select( const QPoint &pos )
{
    double dist = 10e10;
    int index = -1;

    QwtPlotCurve *curve = plot()->getOpaqueTFCurve();
    if(!curve)
        return;

    double d;
    int idx = curve->closestPoint( pos, &d );
    if ( d < dist )
    {
        index = idx;
        dist = d;
    }

    showCursor( false );
    d_selectedPoint = -1;

    if ( dist < 10 ) // 10 pixels tolerance
    {
        d_selectedPoint = index;
        showCursor( true );
    }
}

// Move the selected point
void TFInteraction::moveBy( int dx, int dy )
{
    if ( dx == 0 && dy == 0 )
        return;

    QwtPlotCurve *curve = plot()->getOpaqueTFCurve();
    if(!curve)
        return;

    const QPointF sample =
        curve->sample( d_selectedPoint );

    const double x = plot()->transform(
        curve->xAxis(), sample.x() );
    const double y = plot()->transform(
        curve->yAxis(), sample.y() );

    move( QPoint( qRound( x + dx ), qRound( y + dy ) ) );
}

// Move the selected point
void TFInteraction::move( const QPoint &pos )
{
    QwtPlotCurve *curve = plot()->getOpaqueTFCurve();
    if(!curve || d_selectedPoint == -1)
        return;


    double x,y;
    x=plot()->invTransform( curve->xAxis(), pos.x() );
    y=plot()->invTransform( curve->yAxis(), pos.y() );
    y=y<0?0:y;
    y=y>1?1:y;

    TransferFunction * TF = plot()->getTF();


    TransferFunction::MyIterator it= TF->getIterator_byIndex( d_selectedPoint );
    if (d_selectedPoint==0 || d_selectedPoint==static_cast<int>(curve->dataSize()-1))
    {
        // first and last point is not allowed to move along the x-axis
         it->second.w = y;
    }
    else
    {
        float lowerBoundX = TF->getIterator_byIndex( d_selectedPoint-1 )->first;
        float upBoundX = TF->getIterator_byIndex( d_selectedPoint+1 )->first;
        if (x<=lowerBoundX)
            x=lowerBoundX + TF->delta*(TF->data_max-TF->data_min);
        if (x>=upBoundX)
            x=upBoundX - TF->delta*(TF->data_max-TF->data_min);

        it->first = x;
        it->second.w = y;
    }
    TF->setDirty(true); // to be clean, the modification above should use a TF function. Lazy to do that, trick here
    plot()->updateOpaqueTFPlot();

    plot()->flush();
    emit plot()->TFModified();
}



// create a new point or modify color
void TFInteraction::doubleClicked( const QPoint &pos )
{
    QwtPlotCurve *curve = plot()->getOpaqueTFCurve();
    if(!curve)
        return;

    double dist = 10e10;
    int index = -1;
    double d;
    int idx = curve->closestPoint( pos, &d );
    if ( d < dist )
    {
        index = idx;
        dist = d;
    }
    if (dist<5) // change color
    {
        d_selectedPoint = index;
        Vec4f v= plot()->getTF()->getTFValue_byIndex(index);
        QColor c = QColorDialog::getColor(QColor::fromRgbF(v.x,v.y,v.z) , this->plot() );
        plot()->getTF()->setTFValue_byIndex(index, Vec4f(c.redF(),c.greenF(),c.blueF(),v.w));


    }
    else  // create key point
    {
        double x,y;
        x=plot()->invTransform( curve->xAxis(), pos.x() );
        y=plot()->invTransform( curve->yAxis(), pos.y() );
        plot()->getTF()->addKeypoint(x,y);
        plot()->updateOpaqueTFPlot();
    }

    plot()->flush();
    emit plot()->TFModified();
}

// Hightlight the selected point ---- ignore it
void TFInteraction::showCursor( bool showIt )
{
    plot()->showOpaqueTFPlotColor();
}


