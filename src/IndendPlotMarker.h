
/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef IND_PLOT_MARKER_H
#define IND_PLOT_MARKER_H
#define maxLines 30

#include <qpen.h>
#include <qfont.h>
#include <qstring.h>
#include <qbrush.h>
#include "qwt_global.h"
#include "qwt_plot_item.h"
#include <qwt_plot_marker.h>
/* #include <cstdint> */
#include <stdint.h>

/*!
  \brief A class for drawing markers

  A marker can be a horizontal line, a vertical line,
  a symbol, a label or any combination of them, which can
  be drawn around a center point inside a bounding rectangle.

  The setSymbol() member assigns a symbol to the marker.
  The symbol is drawn at the specified point.

  With setLabel(), a label can be assigned to the marker.
  The setLabelAlignment() member specifies where the label is
  drawn. All the Align*-constants in Qt::AlignmentFlags (see Qt documentation)
  are valid. The interpretation of the alignment depends on the marker's
  line style. The alignment refers to the center point of
  the marker, which means, for example, that the label would be printed
  left above the center point if the alignment was set to 
  Qt::AlignLeft | Qt::AlignTop.

  \note QwtPlotTextLabel is intended to align a text label
        according to the geometry of canvas 
        ( unrelated to plot coordinates )
*/

class QwtIndPlotMarker: public QwtPlotMarker
{
public:

    /*!
        Line styles.
        \sa setLineStyle(), lineStyle()
    */
    /* enum LineStyle */
    /* { */
    /*     //! No line */
    /*     NoLine, */

    /*     //! A horizontal line */
    /*     HLine, */

    /*     //! A vertical line */
    /*     VLine, */

    /*     //! A crosshair */
    /*     Cross */
    /* }; */

    explicit QwtIndPlotMarker( const QString &title = QString::null );
    explicit QwtIndPlotMarker( const QwtText &title );

    virtual ~QwtIndPlotMarker();

    /* virtual int rtti() const; */

    /* double xValue() const; */
    /* double yValue() const; */
    /* QPointF value() const; */

    /* void setXValue( double ); */
    /* void setYValue( double ); */
    /* void setValue( double, double ); */
    /* void setValue( const QPointF & ); */

    /* void setLineStyle( LineStyle st ); */
    /* LineStyle lineStyle() const; */

    /* void setLinePen( const QColor &, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine ); */
    /* void setLinePen( const QPen &p ); */
    /* const QPen &linePen() const; */

    /* void setSymbol( const QwtSymbol * ); */
    /* const QwtSymbol *symbol() const; */

    /* void setLabel( const QwtText& ); */
    /* QwtText label() const; */

    /* void setLabelAlignment( Qt::Alignment ); */
    /* Qt::Alignment labelAlignment() const; */

    /* void setLabelOrientation( Qt::Orientation ); */
    /* Qt::Orientation labelOrientation() const; */

    /* void setSpacing( int ); */
    /* int spacing() const; */

    void setSpacing( int, int );
    void setSpacing( int );
    /* int setSpacingX(); */
    /* int setSpacingY(); */
    int spacingX() const;
    int spacingY() const;
    static void resetDrawnLabels();

    virtual void draw( QPainter *p,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF & ) ;//const;

    /* virtual QRectF boundingRect() const; */

    /* virtual QwtGraphic legendIcon( int index, const QSizeF & ) const; */

protected:
    /* virtual void drawLines( QPainter *, */ 
    /*     const QRectF &, const QPointF & ) const; */

    virtual void drawLabel( QPainter *, 
        const QRectF &, const QPointF & ) const;

private:

    int m_spacingX;
    int m_spacingY;
    /* static bool** drawnPixels; */
    /* static QList<QList <bool> > m_drawnPixels; */
    /* static std::vector< std::vector<bool> > m_drawnPixels; */
    /* static QVector<QVector<bool, MAXLINES> 500> m_drawnPixels; */
    /* class PrivateData; */
    /* PrivateData *d_data; */
    class Matrix {
        public:
            Matrix(unsigned long rows, unsigned long cols);
            // Subscript operators often come in pairs
            bool& operator() (unsigned long row, unsigned long col);        
            // Subscript operators often come in pairs
            bool  operator() (unsigned long row, unsigned long col) const;  
            ~Matrix();                              // Destructor
            Matrix(const Matrix& m);               // Copy constructor
            Matrix& operator= (const Matrix& m);   // Assignment operator
            void set(unsigned long row, unsigned long col, bool value);
            /** init the Memory */
            void init();
            /** reset the Matrix (free memory)*/
            void reset();
            int resize(unsigned long row, unsigned long col);
            int cols();
            int rows();
            uintptr_t canvasId();
            void setCanvasId(uintptr_t canvasId);
            void drawnAt(unsigned long x, unsigned long y, unsigned long xsize, unsigned long ysize);
            bool isFree(unsigned long x, unsigned long y, unsigned long xsize, unsigned long ysize);
            // ...
        private:
            unsigned long m_rows;
            unsigned long m_cols;
            uintptr_t m_canvasId;
            //holds data of where is drawn dimension 1: rows dimension 2: cols
            //e.g. m_data[y*m_rows+cols]
            // y == rows, x == cols
            bool* m_data;
    //TODO: copy constructor, copy assign, move assign constructors
    };
    static Matrix* m_drawnPixels;
};

#endif
