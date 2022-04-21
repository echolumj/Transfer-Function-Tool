#ifndef TFInteraction_H
#define TFInteraction_H

#include <qobject.h>

class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;
class TFWidgetPlot;

class TFInteraction: public QObject
{
    Q_OBJECT
public:
    TFInteraction( TFWidgetPlot *plot );
    virtual bool eventFilter( QObject *, QEvent * );

    //virtual bool event( QEvent * );


private:
    void select( const QPoint & );
    void move( const QPoint & );
    void moveBy( int dx, int dy );
    void deleteSelected();

    void release();

    void showCursor( bool enable );
    //void shiftPointCursor( bool up );

    TFWidgetPlot *plot();
    const TFWidgetPlot *plot() const;

    int d_selectedPoint;
    void doubleClicked(const QPoint &pos);
};

#endif
