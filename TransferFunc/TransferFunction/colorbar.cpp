#include <qevent.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include "colorbar.h"

ColorBar::ColorBar( Qt::Orientation o, QWidget *parent ):
    QWidget( parent ),
    d_orientation( o ),
    d_light( Qt::white ),
    d_dark( Qt::black )
{
#ifndef QT_NO_CURSOR
    setCursor( Qt::PointingHandCursor );
#endif
}

void ColorBar::setOrientation( Qt::Orientation o )
{
    d_orientation = o;
    update();
}

void ColorBar::setLight( const QColor &light )
{
    d_light = light;
    update();
}

void ColorBar::setDark( const QColor &dark )
{
    d_dark = dark;
    update();
}

void ColorBar::setRange( const QColor &light, const QColor &dark )
{
    d_light = light;
    d_dark = dark;
    update();
}

void ColorBar::mousePressEvent( QMouseEvent *e )
{
    if( e->button() ==  Qt::LeftButton )
    {
        // emit the color of the position where the mouse click
        // happened

        const QPixmap pm = QWidget::grab();//QPixmap::grabWidget( this );  lmj★ - 1
        const QRgb rgb = pm.toImage().pixel( e->x(), e->y() );

        Q_EMIT selected( QColor( rgb ) );
        e->accept();
    }
}

void ColorBar::paintEvent( QPaintEvent * )
{
    QPainter painter( this );
    drawColorBar( &painter, rect() );
}

void ColorBar::drawColorBar( QPainter *painter, const QRect &rect ) const
{
    if(!plot->getTF())
        return;
    vector<Vec4f> uniformTF = plot->getTF()->getUniformTF();

    //int h1, s1, v1;
    //int h2, s2, v2;

    //d_light.getHsv( &h1, &s1, &v1 );
    //d_dark.getHsv( &h2, &s2, &v2 );

    painter->save();
    painter->setClipRect( rect );
    painter->setClipping( true );

    //painter->fillRect( rect, d_dark );

    float sectionSize = static_cast<float>(rect.width()) / uniformTF.size(); //(float)rect.width()/uniformTF.size(); lmj★ - 2

    for ( int i = 0; i < static_cast<int>(uniformTF.size()); i++ ) //lmj★ - 3
    {
        QRect section;
        if ( d_orientation == Qt::Horizontal )
        {
            section.setRect( rect.x() + i * sectionSize, rect.y(),
                ceil(sectionSize), rect.height() );
        }
        else
        {
            section.setRect( rect.x(), rect.y() + i * sectionSize,
                rect.width(), sectionSize );
        }

        QColor c = QColor::fromRgbF(uniformTF[i].x,uniformTF[i].y,uniformTF[i].z);
        painter->fillRect( section, c );
    }

    painter->restore();
}

