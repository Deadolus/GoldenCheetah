
/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "IndendPlotMarker.h"
#include "qwt_symbol.h"
#include "qwt_plot_marker.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
/* #include "qwt_text.h" */
/* #include "qwt_math.h" */
/* #include <qpainter.h> */

QwtIndPlotMarker::Matrix* QwtIndPlotMarker::m_drawnPixels = new QwtIndPlotMarker::Matrix(1,1);
/* QwtIndPlotMarker::m_rows = 0; */
/* QwtIndPlotMarker::m_cols = 0; */
/* QwtIndPlotMarker::m_data = nullptr; */


    inline QwtIndPlotMarker::Matrix::Matrix(unsigned long rows, unsigned long cols): 
        m_rows(rows), m_cols(cols)
      //, m_data ← initialized below after the if...throw statement
{
    if (rows == 0 || cols == 0)
        m_data = 0;
    /*     throw BadIndex("Matrix constructor has 0 size"); */
    /* m_rows = rows; */
    /* m_cols = cols; */
    m_data = new bool[rows * cols]();
    init(); //todo: shouldn't be needed...
}

inline QwtIndPlotMarker::Matrix::~Matrix()
{
    reset();
}

inline bool& QwtIndPlotMarker::Matrix::operator() (unsigned long row, unsigned long col)
{
    /* if (row >= m_rows || col >= m_cols) */
    /*     return 0; */
    /*     throw BadIndex("Matrix subscript out of bounds"); */
    if ((row >= m_rows || col >= m_cols)||(!m_data))
        //TODO: not pretty...
        printf("Error\n");
    /* throw std::out_of_range("invalid index"); */

        return m_data[m_rows*row + col];
}

inline bool QwtIndPlotMarker::Matrix::operator() (unsigned long row, unsigned long col) const
{
    if ((row >= m_rows || col >= m_cols)||(!m_data))
        printf("Error\n");
    /* throw std::out_of_range("operator (): invalid index"); */
        /* return false; */
    /*     throw BadIndex("const Matrix subscript out of bounds"); */
    return m_data[m_rows*row + col];
}

inline void QwtIndPlotMarker::Matrix::init() {
    for (unsigned long i = 0; i < m_cols*m_rows; i++) {
        m_data[i]=false;
    }
}

inline void QwtIndPlotMarker::Matrix::reset() {
    delete[] m_data;
    //m_data = nullptr;
    m_data = 0;
    m_rows = 0;
    m_cols = 0;
}

int QwtIndPlotMarker::Matrix::resize(unsigned long row, unsigned long col) {
    delete[] m_data;

    m_data = new bool[row * col]();
    if(m_data) {
        m_rows = row;
        m_cols = col;
    //TODO: should not have to init:
    init();
    printf("-->New size: %lu, %lu\n", m_rows, m_cols);
        return 0;
    }
    else {
        /*couldn't allocate new array TODO: handle better */
        return -1;
    }
}

int QwtIndPlotMarker::Matrix::rows() {
    return m_rows;
}

int QwtIndPlotMarker::Matrix::cols() {
    return m_cols;
}
uintptr_t QwtIndPlotMarker::Matrix::canvasId(){
    return m_canvasId;
}
void QwtIndPlotMarker::Matrix::setCanvasId(uintptr_t vcanvasId){
    m_canvasId = vcanvasId;
}
void QwtIndPlotMarker::Matrix::set(unsigned long row, unsigned long col, bool value) {
    m_data[m_rows*row+col] = value;
}

void QwtIndPlotMarker::Matrix::drawnAt(unsigned long row, unsigned long col, unsigned long ysize, unsigned long xsize){
    printf("drawn: %u, %u:%u, %u", row, col, row+ysize, col+xsize);
    if((row>=m_rows)||(col>=m_cols))
        return;
    for (unsigned long y = row; (y < (ysize+row))&&(y<m_rows); y++) {
        for (unsigned long x = col; (x < (xsize+col))&&(x<m_cols); x++) {
            m_data[m_rows*y+x] = true;
        }
    }
}
bool QwtIndPlotMarker::Matrix::isFree(unsigned long row, unsigned long col, unsigned long ysize, unsigned long xsize){
    printf("isfree: %u, %u:%u, %u", row, col, row+ysize, col+xsize);
    bool ret = true;
    if((row>=m_rows)||(col>=m_cols))
        return ret;
    for (unsigned long y = row; (y < (ysize+row))&&ret&&(y<m_rows);y++) {
        for (unsigned long x = col; (x < (xsize+col))&&ret&&(x<m_cols); x++) {
            ret = !(m_data[m_rows*y+x]);
        }
    }
    return ret;
}
    /* class QwtIndPlotMarker::PrivateData */
    /* { */
    /* public: */
    /*     PrivateData(): */
    /*         labelAlignment( Qt::AlignCenter ), */
    /*         labelOrientation( Qt::Horizontal ), */
    /*         spacing( 2 ), */
    /*         symbol( NULL ), */
    /*         style( QwtIndPlotMarker::NoLine ), */
    /*         xValue( 0.0 ), */
    /*         yValue( 0.0 ) */
    /*     { */
    /*     } */

    /*     ~PrivateData() */
    /*     { */
    /*         delete symbol; */
    /*     } */

    /*     QwtText label; */
    /*     Qt::Alignment labelAlignment; */
    /*     Qt::Orientation labelOrientation; */
    /*     int spacing; */

    /*     QPen pen; */
    /*     const QwtSymbol *symbol; */
    /*     LineStyle style; */

    /*     double xValue; */
    /*     double yValue; */
    /* }; */

    //! Sets alignment to Qt::AlignCenter, and style to QwtIndPlotMarker::NoLine
    QwtIndPlotMarker::QwtIndPlotMarker( const QString &title ):
        QwtPlotMarker(QwtText(title))
    {
    m_spacingX=0; 
    m_spacingY=0;
    if(!this->m_drawnPixels) {
        this->m_drawnPixels = new Matrix(20,20);
    }
    /* m_drawnPixels.reset(); */
    /* for (int i = 0; i < 30; i++) { */
    /*     m_drawnPixels.append(new QList<bool>()); */
    /*     m_drawnPixels[i].push_back(false); */
    /* } */
            /* drawnDates = new bool*[daysOffset]; */
            /* for (int i = 0; i < daysOffset; ++i) { */

            /*     drawnDates[i]=new bool[maxDepth](); */
            /* } */
    }

    //! Sets alignment to Qt::AlignCenter, and style to QwtIndPlotMarker::NoLine
QwtIndPlotMarker::QwtIndPlotMarker( const QwtText &title ):
    QwtPlotMarker(title)
{
    m_spacingX=0;
    m_spacingY=0;
    if(!m_drawnPixels) {
        m_drawnPixels = new Matrix(20,20);
    }
}

    //! Destructor
    QwtIndPlotMarker::~QwtIndPlotMarker()
    {
            /* for (int i = 0; i < daysOffset; ++i) { */

            /*     delete[]drawnDates[i]; */
            /* } */
            /* delete[] drawnDates; */
    }

    //! \return QwtPlotItem::Rtti_PlotMarker
    /* int QwtIndPlotMarker::rtti() const */
    /* { */
    /*     return QwtPlotItem::Rtti_PlotMarker; */
    /* } */

    //! Return Value
    /* QPointF QwtIndPlotMarker::value() const */
    /* { */
    /*     return QPointF( d_data->xValue, d_data->yValue ); */
    /* } */

    //! Return x Value
    /* double QwtIndPlotMarker::xValue() const */
    /* { */
    /*     return d_data->xValue; */
    /* } */

    //! Return y Value
    /* double QwtIndPlotMarker::yValue() const */
    /* { */
    /*     return d_data->yValue; */
/* } */

//! Set Value
/* void QwtIndPlotMarker::setValue( const QPointF& pos ) */
/* { */
/*     setValue( pos.x(), pos.y() ); */
/* } */

//! Set Value
/* void QwtIndPlotMarker::setValue( double x, double y ) */
/* { */
/*     if ( x != d_data->xValue || y != d_data->yValue ) */
/*     { */
/*         d_data->xValue = x; */
/*         d_data->yValue = y; */
/*         itemChanged(); */
/*     } */
/* } */

//! Set X Value
/* void QwtIndPlotMarker::setXValue( double x ) */
/* { */
/*     setValue( x, d_data->yValue ); */
/* } */

//! Set Y Value
/* void QwtIndPlotMarker::setYValue( double y ) */
/* { */
/*     setValue( d_data->xValue, y ); */
/* } */

/*!
  Draw the marker

  \param painter Painter
  \param xMap x Scale Map
  \param yMap y Scale Map
  \param canvasRect Contents rectangle of the canvas in painter coordinates
*/
void QwtIndPlotMarker::draw( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect ) //const
{
    const QPointF pos( xMap.transform( this->xValue() ), 
        yMap.transform( this->yValue() ) );

    // draw lines

    drawLines( painter, canvasRect, pos );

    // draw symbol
    if ( this->symbol() &&
        ( this->symbol()->style() != QwtSymbol::NoSymbol ) )
    {
        const QSizeF sz = this->symbol()->size();

        const QRectF clipRect = canvasRect.adjusted( 
            -sz.width(), -sz.height(), sz.width(), sz.height() );

        if ( clipRect.contains( pos ) )
            this->symbol()->drawSymbol( painter, pos );
    }

    drawLabel( painter, canvasRect, pos );
}

/*!
  Draw the lines marker

  \param painter Painter
  \param canvasRect Contents rectangle of the canvas in painter coordinates
  \param pos Position of the marker, translated into widget coordinates

  \sa drawLabel(), QwtSymbol::drawSymbol()
*/
/* void QwtIndPlotMarker::drawLines( QPainter *painter, */
/*     const QRectF &canvasRect, const QPointF &pos ) const */
/* { */
/*     if ( this->style() == NoLine ) */
/*         return; */

/*     const bool doAlign = QwtPainter::roundingAlignment( painter ); */

/*     painter->setPen( d_data->pen ); */
/*     if ( d_data->style == QwtIndPlotMarker::HLine || */
/*         d_data->style == QwtIndPlotMarker::Cross ) */
/*     { */
/*         double y = pos.y(); */
/*         if ( doAlign ) */
/*             y = qRound( y ); */

/*         QwtPainter::drawLine( painter, canvasRect.left(), */
/*             y, canvasRect.right() - 1.0, y ); */
/*     } */
/*     if ( d_data->style == QwtIndPlotMarker::VLine || */
/*         d_data->style == QwtIndPlotMarker::Cross ) */
/*     { */
/*         double x = pos.x(); */
/*         if ( doAlign ) */
/*             x = qRound( x ); */

/*         QwtPainter::drawLine( painter, x, */
/*             canvasRect.top(), x, canvasRect.bottom() - 1.0 ); */
/*     } */
/* } */

/*!
  Align and draw the text label of the marker

  \param painter Painter
  \param canvasRect Contents rectangle of the canvas in painter coordinates
  \param pos Position of the marker, translated into widget coordinates

  \sa drawLabel(), QwtSymbol::drawSymbol()
*/
void QwtIndPlotMarker::drawLabel( QPainter *painter,
    const QRectF &canvasRect, const QPointF &pos ) const
{
        printf("canvas: %lu\n", reinterpret_cast<uintptr_t>(&canvasRect));
    if(reinterpret_cast<uintptr_t>(painter)!=m_drawnPixels->canvasId()){
        m_drawnPixels->setCanvasId(reinterpret_cast<uintptr_t>(painter));
        m_drawnPixels->init();
        printf("new painter: %lu\n", m_drawnPixels->canvasId());
    }
    /* printf("%d, %d\n", reinterpret_cast<uintptr_t>(painter), reinterpret_cast<uintptr_t>(&canvasRect)); */
    printf("in drawLabel %lf, %lf\n",canvasRect.height(),canvasRect.width());
    if(((*m_drawnPixels).cols()!=canvasRect.width())||((*m_drawnPixels).rows()!=canvasRect.height())){
m_drawnPixels->resize(canvasRect.height(), canvasRect.width());
            }
    if ( this->label().isEmpty() )
        return;

    Qt::Alignment align = labelAlignment();
    QPointF alignPos = pos;

    QSizeF symbolOff( 0, 0 );

    switch ( this->lineStyle() )
    {
        case QwtPlotMarker::VLine:
        {
            // In VLine-style the y-position is pointless and
            // the alignment flags are relative to the canvas

            if ( this->labelAlignment() & Qt::AlignTop )
            {
                alignPos.setY( canvasRect.top() );
                align &= ~Qt::AlignTop;
                align |= Qt::AlignBottom;
            }
            else if ( this->labelAlignment() & Qt::AlignBottom )
            {
                // In HLine-style the x-position is pointless and
                // the alignment flags are relative to the canvas

                alignPos.setY( canvasRect.bottom() - 1 );
                align &= ~Qt::AlignBottom;
                align |= Qt::AlignTop;
            }
            else
            {
                alignPos.setY( canvasRect.center().y() );
            }
            break;
        }
        case QwtPlotMarker::HLine:
        {
            if ( this->labelAlignment() & Qt::AlignLeft )
            {
                alignPos.setX( canvasRect.left() );
                align &= ~Qt::AlignLeft;
                align |= Qt::AlignRight;
            }
            else if ( this->labelAlignment() & Qt::AlignRight )
            {
                alignPos.setX( canvasRect.right() - 1 );
                align &= ~Qt::AlignRight;
                align |= Qt::AlignLeft;
            }
            else
            {
                alignPos.setX( canvasRect.center().x() );
            }
            break;
        }
        default:
        {
            if ( this->symbol() &&
                ( this->symbol()->style() != QwtSymbol::NoSymbol ) )
            {
                symbolOff = this->symbol()->size() + QSizeF( 1, 1 );
                symbolOff /= 2;
            }
        }
    }

    qreal pw2 = this->linePen().widthF() / 2.0;
    if ( pw2 == 0.0 )
        pw2 = 0.5;

    /* const int spacing = this->spacing(); */
    const int spacingX = m_spacingX;
    const int spacingY = m_spacingY;

    const qreal xOff = qMax( pw2, symbolOff.width() );
    const qreal yOff = qMax( pw2, symbolOff.height() );

    const QSizeF textSize = this->label().textSize( painter->font() );

    if ( align & Qt::AlignLeft )
    {
        alignPos.rx() -= xOff + spacingX;
        if ( this->labelOrientation() == Qt::Vertical )
            alignPos.rx() -= textSize.height();
        else
            alignPos.rx() -= textSize.width();
    }
    else if ( align & Qt::AlignRight )
    {
        alignPos.rx() += xOff + spacingX;
        /* m_drawnPixels[0][0] = m_drawnPixels[0][0] ? false : true; */
    }
    else
    {
        if ( this->labelOrientation() == Qt::Vertical )
            alignPos.rx() -= textSize.height() / 2;
        else
            alignPos.rx() -= textSize.width() / 2;
    }

    if ( align & Qt::AlignTop )
    {
        printf("in top\n");
        alignPos.ry() -= yOff + spacingY;
       /* m_drawnPixels->set(0,0, (*m_drawnPixels)(0,0) ? false : true); */
        if ( this->labelOrientation() != Qt::Vertical )
            alignPos.ry() -= textSize.height();
    }
    else if ( align & Qt::AlignBottom )
    {
        printf("in bottom\n");
        //TODO: remove spacing var, replace with both spacing> and X
        alignPos.ry() += yOff + spacingY;
        /* m_drawnPixels->set(0,0, (*m_drawnPixels)(0,0) ? false : true); */
        if ( this->labelOrientation() == Qt::Vertical )
            alignPos.ry() += textSize.width();
    }
    else
    {
        if ( this->labelOrientation() == Qt::Vertical )
            alignPos.ry() += textSize.width() / 2;
        else
            alignPos.ry() -= textSize.height() / 2;
    }
    //Check to see if the calculated space is free, else shift
    while(!m_drawnPixels->isFree(alignPos.y(),alignPos.x(),textSize.height(),textSize.width())) {
        alignPos.ry() += textSize.height();
        printf("indend\n");
    }
    painter->translate( alignPos.x(), alignPos.y() );
    if ( this->labelOrientation() == Qt::Vertical )
        painter->rotate( -90.0 );

    const QRectF textRect( 0, 0, textSize.width(), textSize.height() );
    this->label().draw( painter, textRect );
    printf("textrect %f, %f\n", textSize.width(), textSize.height());
    printf("alignPos %f, %f\n", alignPos.x(), alignPos.y());
    m_drawnPixels->drawnAt(alignPos.y(),alignPos.x(),textSize.height(),textSize.width());
printf("label %s \n\n",this->label().text().toStdString().c_str());
}

/*!
  \brief Set the line style
  \param style Line style. 
  \sa lineStyle()
*/
/* void QwtIndPlotMarker::setLineStyle( LineStyle style ) */
/* { */
/*     if ( style != d_data->style ) */
/*     { */
/*         d_data->style = style; */

/*         legendChanged(); */
/*         itemChanged(); */
/*     } */
/* } */

/*!
  \return the line style
  \sa setLineStyle()
*/
/* QwtIndPlotMarker::LineStyle QwtIndPlotMarker::lineStyle() const */
/* { */
/*     return d_data->style; */
/* } */

/*!
  \brief Assign a symbol
  \param symbol New symbol
  \sa symbol()
*/
/* void QwtIndPlotMarker::setSymbol( const QwtSymbol *symbol ) */
/* { */
/*     if ( symbol != d_data->symbol ) */
/*     { */
/*         delete d_data->symbol; */
/*         d_data->symbol = symbol; */

/*         if ( symbol ) */
/*             setLegendIconSize( symbol->boundingRect().size() ); */

/*         legendChanged(); */
/*         itemChanged(); */
/*     } */
/* } */

/*!
  \return the symbol
  \sa setSymbol(), QwtSymbol
*/
/* const QwtSymbol *QwtIndPlotMarker::symbol() const */
/* { */
/*     return d_data->symbol; */
/* } */

/*!
  \brief Set the label
  \param label Label text
  \sa label()
*/
/* void QwtIndPlotMarker::setLabel( const QwtText& label ) */
/* { */
/*     if ( label != d_data->label ) */
/*     { */
/*         d_data->label = label; */
/*         itemChanged(); */
/*     } */
/* } */

/*!
  \return the label
  \sa setLabel()
*/
/* QwtText QwtIndPlotMarker::label() const */
/* { */
/*     return d_data->label; */
/* } */

/*!
  \brief Set the alignment of the label

  In case of QwtIndPlotMarker::HLine the alignment is relative to the
  y position of the marker, but the horizontal flags correspond to the
  canvas rectangle. In case of QwtIndPlotMarker::VLine the alignment is
  relative to the x position of the marker, but the vertical flags
  correspond to the canvas rectangle.

  In all other styles the alignment is relative to the marker's position.

  \param align Alignment. 
  \sa labelAlignment(), labelOrientation()
*/
/* void QwtIndPlotMarker::setLabelAlignment( Qt::Alignment align ) */
/* { */
/*     if ( align != d_data->labelAlignment ) */
/*     { */
/*         d_data->labelAlignment = align; */
/*         itemChanged(); */
/*     } */
/* } */

/*!
  \return the label alignment
  \sa setLabelAlignment(), setLabelOrientation()
*/
/* Qt::Alignment QwtIndPlotMarker::labelAlignment() const */
/* { */
/*     return d_data->labelAlignment; */
/* } */

/*!
  \brief Set the orientation of the label

  When orientation is Qt::Vertical the label is rotated by 90.0 degrees
  ( from bottom to top ).

  \param orientation Orientation of the label

  \sa labelOrientation(), setLabelAlignment()
*/
/* void QwtIndPlotMarker::setLabelOrientation( Qt::Orientation orientation ) */
/* { */
/*     if ( orientation != d_data->labelOrientation ) */
/*     { */
/*         d_data->labelOrientation = orientation; */
/*         itemChanged(); */
/*     } */
/* } */

/*!
  \return the label orientation
  \sa setLabelOrientation(), labelAlignment()
*/
/* Qt::Orientation QwtIndPlotMarker::labelOrientation() const */
/* { */
/*     return d_data->labelOrientation; */
/* } */

/*!
  \brief Set the spacing

  When the label is not centered on the marker position, the spacing
  is the distance between the position and the label.

  \param spacing Spacing
  \sa spacing(), setLabelAlignment()
*/
void QwtIndPlotMarker::setSpacing( int spacing )
{
    if ( spacing < 0 )
        spacing = 0;

    if (( spacing == this->spacingX() )&&( spacing == this->spacingY() ))
        return;

    m_spacingX = spacing;
    m_spacingY = spacing;

    itemChanged();
}

/*!
  \brief Set the spacing

  When the label is not centered on the marker position, the spacing
  is the distance between the position and the label.

  \param spacing Spacing
  \sa spacing(), setLabelAlignment()
*/
void QwtIndPlotMarker::setSpacing( int spacingX, int spacingY )
{
    if ( spacingX < 0 )
        spacingX = 0;
    if ( spacingY < 0 )
        spacingY = 0;


    if (( spacingX == this->spacingX() )||( spacingY == this->spacingY() ))
        return;

    m_spacingX = spacingX;
    m_spacingY = spacingY;

    itemChanged();
}

/*!
  \return the spacing
  \sa setSpacing()
*/
/* int QwtIndPlotMarker::spacing() const */
/* { */
/*     return d_data->spacing; */
/* } */

/*!
  \return the spacing X
  \sa setSpacing()
*/
int QwtIndPlotMarker::spacingX() const
{
    return m_spacingX;
}

/*!
  \return the spacing Y
  \sa setSpacing()
*/
int QwtIndPlotMarker::spacingY() const
{
    return m_spacingY;
}

void QwtIndPlotMarker::resetDrawnLabels(){
   m_drawnPixels->init(); 
    /* for (int i = 0; i < m_drawnPixels.size(); i++) { */
    /*     for (int j = 0; j < m_drawnPixels[i].size(); j++) { */
    /*         m_drawnPixels[i][j]=false; */
    /*     } */
    /* } */
}

/*! 
  Build and assign a line pen
    
  In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
  non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
  to hide this incompatibility.
    
  \param color Pen color
  \param width Pen width
  \param style Pen style
    
  \sa pen(), brush()
 */ 
/* void QwtIndPlotMarker::setLinePen( const QColor &color, qreal width, Qt::PenStyle style ) */
/* { */   
/*     setLinePen( QPen( color, width, style ) ); */
/* } */

/*!
  Specify a pen for the line.

  \param pen New pen
  \sa linePen()
*/
/* void QwtIndPlotMarker::setLinePen( const QPen &pen ) */
/* { */
/*     if ( pen != d_data->pen ) */
/*     { */
/*         d_data->pen = pen; */

/*         legendChanged(); */
/*         itemChanged(); */
/*     } */
/* } */

/*!
  \return the line pen
  \sa setLinePen()
*/
/* const QPen &QwtIndPlotMarker::linePen() const */
/* { */
/*     return d_data->pen; */
/* } */

/* QRectF QwtIndPlotMarker::boundingRect() const */
/* { */
/*     return QRectF( d_data->xValue, d_data->yValue, 0.0, 0.0 ); */
/* } */

/*!
   \return Icon representing the marker on the legend

   \param index Index of the legend entry 
                ( usually there is only one )
   \param size Icon size

   \sa setLegendIconSize(), legendData()
*/
/* QwtGraphic QwtIndPlotMarker::legendIcon( int index, */
/*     const QSizeF &size ) const */
/* { */
/*     Q_UNUSED( index ); */

/*     if ( size.isEmpty() ) */
/*         return QwtGraphic(); */

/*     QwtGraphic icon; */
/*     icon.setDefaultSize( size ); */
/*     icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true ); */

/*     QPainter painter( &icon ); */
/*     painter.setRenderHint( QPainter::Antialiasing, */
/*         testRenderHint( QwtPlotItem::RenderAntialiased ) ); */

/*     if ( d_data->style != QwtIndPlotMarker::NoLine ) */
/*     { */
/*         painter.setPen( d_data->pen ); */

/*         if ( d_data->style == QwtIndPlotMarker::HLine || */
/*             d_data->style == QwtIndPlotMarker::Cross ) */
/*         { */
/*             const double y = 0.5 * size.height(); */

/*             QwtPainter::drawLine( &painter, */ 
/*                 0.0, y, size.width(), y ); */
/*         } */

/*         if ( d_data->style == QwtIndPlotMarker::VLine || */
/*             d_data->style == QwtIndPlotMarker::Cross ) */
/*         { */
/*             const double x = 0.5 * size.width(); */

/*             QwtPainter::drawLine( &painter, */ 
/*                 x, 0.0, x, size.height() ); */
/*         } */
/*     } */

/*     if ( d_data->symbol ) */
/*     { */
/*         const QRect r( 0.0, 0.0, size.width(), size.height() ); */
/*         d_data->symbol->drawSymbol( &painter, r ); */
/*     } */

/*     return icon; */
/* } */

