// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////////////
//
// WorldPlot
//
// Class for transforming world coords into pixel space.
// Actual drawing is performed in pixel space
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <iostream>
#include <cstdio>
#include <QtCore/QLineF>
#include <QtGui/QPainterPath>

#include <toolsa/toolsa_macros.h>
#include "WorldPlot.hh"
#include "ColorMap.hh"
using namespace std;

////////////////////////////////////////
// default constructor

WorldPlot::WorldPlot()
{

  _widthPixels = 100;
  _heightPixels = 100;
  
  _xPixOffset = 0;
  _yPixOffset = 0;

  _leftMargin = 0;
  _rightMargin = 0;
  _topMargin = 0;
  _bottomMargin = 0;
  _axisTextMargin = 0;
  _legendTextMargin = 0;

  _colorScaleWidth = 0;

  _xAxisTickLen = 7;
  _xNTicksIdeal = 7;
  _xSpecifyTicks = false;
  _xTickMin = 0;
  _xTickDelta = 1;
  
  _yAxisTickLen = 7;
  _yNTicksIdeal = 7;
  _ySpecifyTicks = false;
  _yTickMin = 0;
  _yTickDelta = 1;

  _xAxisLabelsInside = true;
  _yAxisLabelsInside = true;

  _titleFontSize = 9;
  _axisLabelFontSize = 7;
  _tickValuesFontSize = 7;
  _legendFontSize = 7;

  _titleColor = "white";
  _axisLineColor = "white";
  _axisTextColor = "white";
  _gridColor = "gray";
  
  _xMinWorld = 0;
  _xMaxWorld = 1;
  _yMinWorld = 0;
  _yMaxWorld = 1;

  _computeTransform();
  
}

////////////////////////////////////////
// copy constructor

WorldPlot::WorldPlot(const WorldPlot &rhs)

{
  _copy(rhs);
}

/////////////////////////////
// Assignment
//

WorldPlot &WorldPlot::operator=(const WorldPlot &rhs)
  
{
  return _copy(rhs);
}

////////////////////////////////////////
// copy method

WorldPlot &WorldPlot::_copy(const WorldPlot &rhs)

{
    
  if (&rhs == this) {
    return *this;
  }

  // copy the meta data

  _widthPixels = rhs._widthPixels;
  _heightPixels = rhs._heightPixels;
  
  _xPixOffset = rhs._xPixOffset;
  _yPixOffset = rhs._yPixOffset;

  _leftMargin = rhs._leftMargin;
  _rightMargin = rhs._rightMargin;
  _topMargin = rhs._topMargin;
  _bottomMargin = rhs._bottomMargin;
  _axisTextMargin = rhs._axisTextMargin;
  _legendTextMargin = rhs._legendTextMargin;

  _colorScaleWidth = rhs._colorScaleWidth;
  
  _xAxisTickLen = rhs._xAxisTickLen;
  _xNTicksIdeal = rhs._xNTicksIdeal;
  _xSpecifyTicks = rhs._xSpecifyTicks;
  _xTickMin = rhs._xTickMin;
  _xTickDelta = rhs._xTickDelta;

  _yAxisTickLen = rhs._yAxisTickLen;
  _yNTicksIdeal = rhs._yNTicksIdeal;
  _ySpecifyTicks = rhs._ySpecifyTicks;
  _yTickMin = rhs._yTickMin;
  _yTickDelta = rhs._yTickDelta;

  _xAxisLabelsInside = rhs._xAxisLabelsInside;
  _yAxisLabelsInside = rhs._yAxisLabelsInside;

  _titleFontSize = rhs._titleFontSize;
  _axisLabelFontSize = rhs._axisLabelFontSize;
  _tickValuesFontSize = rhs._tickValuesFontSize;
  _legendFontSize = rhs._legendFontSize;

  _backgroundColor = rhs._backgroundColor;
  _titleColor = rhs._titleColor;
  _axisLineColor = rhs._axisLineColor;
  _axisTextColor = rhs._axisTextColor;
  _gridColor = rhs._gridColor;

  _xMinWorld = rhs._xMinWorld;
  _xMaxWorld = rhs._xMaxWorld;
  _yMinWorld = rhs._yMinWorld;
  _yMaxWorld = rhs._yMaxWorld;
  
  _plotWidth = rhs._plotWidth;
  _plotHeight = rhs._plotHeight;

  _xMinPixel = rhs._xMinPixel;
  _xMaxPixel = rhs._xMaxPixel;
  _yMinPixel = rhs._yMinPixel;
  _yMaxPixel = rhs._yMaxPixel;

  _xPixelsPerWorld = rhs._xPixelsPerWorld;
  _yPixelsPerWorld = rhs._yPixelsPerWorld;
  
  _xMinWindow = rhs._xMinWindow;
  _xMaxWindow = rhs._xMaxWindow;
  _yMinWindow = rhs._yMinWindow;
  _yMaxWindow = rhs._yMaxWindow;

  _topTicks = rhs._topTicks;
  _bottomTicks = rhs._bottomTicks;
  _leftTicks = rhs._leftTicks;
  _rightTicks = rhs._rightTicks;

  _computeTransform();

  return *this;

}

///////////////////////////////////////////////////////
// set size and location of plotting window
// within the main canvas
// side effect - recomputes transform

void WorldPlot::setWindowGeom(int width,
                              int height,
                              int xOffset,
                              int yOffset)
{

  _widthPixels = width;
  _heightPixels = height;
  _xPixOffset = xOffset;
  _yPixOffset = yOffset;

  _computeTransform();

}

///////////////////////////////////////////////////////
// set world coord limits for window
// side effect - recomputes transform

void WorldPlot::setWorldLimits(double xMinWorld,
                               double yMinWorld,
                               double xMaxWorld,
                               double yMaxWorld)
  
{
  
  if (_xMinWorld < _xMaxWorld) {
    _xMinWorld = MIN(xMinWorld, xMaxWorld);
    _xMaxWorld = MAX(xMinWorld, xMaxWorld);
  } else {
    _xMinWorld = MAX(xMinWorld, xMaxWorld);
    _xMaxWorld = MIN(xMinWorld, xMaxWorld);
  }
    
  if (_yMinWorld < _yMaxWorld) {
    _yMinWorld = MIN(yMinWorld, yMaxWorld);
    _yMaxWorld = MAX(yMinWorld, yMaxWorld);
  } else {
    _yMinWorld = MAX(yMinWorld, yMaxWorld);
    _yMaxWorld = MIN(yMinWorld, yMaxWorld);
  }

  _computeTransform();

}

///////////////////////////////////////////////////////
// set zoom limits from pixel space

void WorldPlot::setZoomLimits(int xMin,
                              int yMin,
                              int xMax,
                              int yMax)
  
{
  setWorldLimits(getXWorld(xMin),
                 getYWorld(yMin),
                 getXWorld(xMax),
                 getYWorld(yMax));
}

void WorldPlot::setZoomLimitsX(int xMin,
                               int xMax)
  
{
  setWorldLimits(getXWorld(xMin),
                 _yMinWorld,
                 getXWorld(xMax),
                 _yMaxWorld);
}

void WorldPlot::setZoomLimitsY(int yMin,
                               int yMax)
  
{
  setWorldLimits(_xMinWorld,
                 getYWorld(yMin),
                 _xMaxWorld,
                 getYWorld(yMax));
}

////////////////////////////////////////
// resize the plot

void WorldPlot::resize(int width, 
                       int height)

{
    
  _widthPixels = width;
  _heightPixels = height;
  
  _computeTransform();

}

////////////////////////////////////////
// set the offsets

void WorldPlot::setWindowOffsets(int xOffset,
                                 int yOffset)
{

  _xPixOffset = xOffset;
  _yPixOffset = yOffset;

  _computeTransform();

}

//////////////////////////////////////////////////////////////////////////
// set the y scale from the x scale - for plotting with aspect ration 1.0

void WorldPlot::setYscaleFromXscale() 

{
  
  if (_yPixelsPerWorld * _xPixelsPerWorld < 0) {
    _yPixelsPerWorld = _xPixelsPerWorld * -1.0;
  } else {
    _yPixelsPerWorld = _xPixelsPerWorld;
  }

  double yMean = (_yMaxWorld + _yMinWorld) / 2.0;
  double yHalf = ((_yMaxPixel - _yMinPixel) / 2.0) / _yPixelsPerWorld;
  _yMinWorld = yMean - yHalf; 
  _yMaxWorld = yMean + yHalf;

  _computeTransform();
	
}

/////////////////////////////////////////////////////////////////////
// get the rectangle that describes the full window in world coords

QRect WorldPlot::getWorldWindow() const

{

  QRect rect(floor(_xMinWindow * 10000.0 + 0.5),
             floor(_yMinWindow * 10000.0 + 0.5),
             floor((_xMaxWindow - _xMinWindow) * 10000.0 + 0.5),
             floor((_yMaxWindow - _yMinWindow) * 10000.0) + 0.5);

  return rect;

}
  
///////////////////////////////
// draw a line in pixel coords

void WorldPlot::drawPixelLine(QPainter &painter,
                              double xx1, double yy1,
                              double xx2, double yy2) 

{
    
  QLineF qline(xx1, yy1, xx2, yy2);
  painter.drawLine(qline);
    
}

///////////////////////////////
// draw a line in world coords

void WorldPlot::drawLine(QPainter &painter,
                         double x1, double y1,
                         double x2, double y2) 

{
	
  painter.save();
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);

  double xx1 = getXPixel(x1);
  double yy1 = getYPixel(y1);
  double xx2 = xx1 + (x2 - x1) * _xPixelsPerWorld;
  double yy2 = yy1 + (y2 - y1) * _yPixelsPerWorld;

  QLineF qline(xx1, yy1, xx2, yy2);
  painter.drawLine(qline);

  painter.restore();

}

////////////////////////////////////////////////////////////////////////
// draw lines between consecutive points, world coords

void WorldPlot::drawLines(QPainter &painter, QVector<QPointF> &points) 

{

  painter.save();
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);

  QPainterPath path;

  double xx0 = getXPixel(points[0].x());
  double yy0 = getYPixel(points[0].y());

  path.moveTo(xx0, yy0);

  QVector<QLineF> lines;
    
  for (int ii = 1; ii < points.size(); ii++) {

    double xx = getXPixel(points[ii].x());
    double yy = getYPixel(points[ii].y());
      
    path.lineTo(xx, yy);

  }

  painter.drawPath(path);
  painter.restore();
}

/////////////////////////
// draw a rectangle
  
void WorldPlot::drawRectangle(QPainter &painter,
                              double x, double y,
                              double w, double h) 

{

  double xx = getXPixel(x);
  double yy = getYPixel(y);
  double width = w * _xPixelsPerWorld;
  double height = h * _yPixelsPerWorld * -1.0;
  yy -= height;

  QRectF rect(xx, yy, width, height);

  painter.drawRect(rect);

}

///////////////////
// fill a rectangle

void WorldPlot::fillRectangle(QPainter &painter,
                              QBrush &brush,
                              double x, double y,
                              double w, double h) 

{
    
  double xx = getXPixel(x);
  double yy = getYPixel(y);
  double width = w * _xPixelsPerWorld;
  double height = h * _yPixelsPerWorld * -1.0;
  yy -= height;

  painter.fillRect(xx, yy, width, height, brush);

}

///////////////////
// fill a trapezium

void WorldPlot::fillTrap(QPainter &painter,
                         QBrush &brush,
                         double x0, double y0,
                         double x1, double y1,
                         double x2, double y2,
                         double x3, double y3)
  
{

  painter.save();
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);
  
  // create a vector of points

  QVector<QPointF> pts;
  QPointF pt0(getXPixel(x0), getYPixel(y0));
  pts.push_back(pt0);
  QPointF pt1(getXPixel(x1), getYPixel(y1));
  pts.push_back(pt1);
  QPointF pt2(getXPixel(x2), getYPixel(y2));
  pts.push_back(pt2);
  QPointF pt3(getXPixel(x3), getYPixel(y3));
  pts.push_back(pt3);
  pts.push_back(pt0); // close

  // create a polygon from the points
  QPolygonF poly(pts);

  // add the polygon to a painter path
  QPainterPath path;
  path.addPolygon(poly);

  // fill the path

  painter.fillPath(path, brush);
  // drawPath(painter, path);

  painter.restore();

}

//////////////
// draw an arc

void WorldPlot::drawArc(QPainter &painter,
                        double x, double y,
                        double w, double h,
                        double startAngle, double arcAngle) 
{
	
  double xx = getXPixel(x);
  double yy = getYPixel(y);
  double ww = w * _xPixelsPerWorld;
  double hh = h * _yPixelsPerWorld * -1.0;

  painter.drawArc(xx, yy, ww, hh, startAngle, arcAngle);

}

//////////////////////
// draw a general path

void WorldPlot::drawPath(QPainter &painter, QPainterPath &path) 

{

  QPainterPath opath;
    
  for (int ii = 0; ii < path.length(); ii++) {

    const QPainterPath::Element &el = path.elementAt(ii);

    double xx = getXPixel(el.x);
    double yy = getYPixel(el.y);

    if (el.isLineTo()) {
      opath.lineTo(xx, yy);
    } else if (el.isMoveTo()) {
      opath.moveTo(xx, yy);
    }
      
  }

  painter.drawPath(path);

}

////////////////////////////////////////////////////
// draw a general path clipped to within the margins

void WorldPlot::drawPathClipped(QPainter &painter, QPainterPath &path)

{

  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);
  drawPath(painter, path);
  painter.setClipping(false);

}

///////////////////////////////////////////////////////////////////////////
// Text
//
// Flags give the justification in Qt, and are or'd from the following:
//    Qt::AlignLeft aligns to the left border.
//    Qt::AlignRight aligns to the right border.
//    Qt::AlignJustify produces justified text.
//    Qt::AlignHCenter aligns horizontally centered.
//    Qt::AlignTop aligns to the top border.
//    Qt::AlignBottom aligns to the bottom border.
//    Qt::AlignVCenter aligns vertically centered
//    Qt::AlignCenter (== Qt::AlignHCenter | Qt::AlignVCenter)
//    Qt::TextSingleLine ignores newline characters in the text.
//    Qt::TextExpandTabs expands tabs (see below)
//    Qt::TextShowMnemonic interprets "&x" as x; i.e., underlined.
//    Qt::TextWordWrap breaks the text to fit the rectangle.

// draw text in world coords
    
void WorldPlot::drawText(QPainter &painter, const string &text,
                         double text_x, double text_y,
                         int flags)

{

  int ixx = getIxPixel(text_x);
  int iyy = getIyPixel(text_y);
	
  QRect tRect(painter.fontMetrics().tightBoundingRect(text.c_str()));
  QRect bRect(painter.fontMetrics().boundingRect(ixx, iyy,
                                    tRect.width() + 2, tRect.height() + 2,
                                    flags, text.c_str()));
    
  painter.drawText(bRect, flags, text.c_str());
    
}

/////////////////////////////////////
// draw rotated text in world coords
    
void WorldPlot::drawRotatedText(QPainter &painter, const string &text,
                                double text_x, double text_y,
                                int flags, double rotationDeg) 

{
  
  int ixx = getIxPixel(text_x);
  int iyy = getIyPixel(text_y);
  
  painter.save();
  painter.translate(ixx, iyy);
  painter.rotate(rotationDeg);

  QRect tRect(painter.fontMetrics().tightBoundingRect(text.c_str()));
  QRect bRect(painter.fontMetrics().boundingRect(0, 0,
                                    tRect.width() + 2, tRect.height() + 2,
                                    flags, text.c_str()));
    
  painter.drawText(bRect, flags, text.c_str());

  painter.restore();
    
}

/////////////////////
// draw centered text

void WorldPlot::drawTextCentered(QPainter &painter, const string &text,
                                 double text_x, double text_y) 

{

  drawText(painter, text, text_x, text_y, Qt::AlignCenter);

}

/////////////////////////////////////////////////////////////	
// Title
    
void WorldPlot::drawTitleTopCenter(QPainter &painter,
                                   const string &title) 

{

  // get bounding rectangle
  
  QRect tRect(painter.fontMetrics().tightBoundingRect(title.c_str()));
  
  qreal xx = (qreal) ((_xMinPixel + _xMaxPixel - tRect.width()) / 2.0);
  qreal yy = (qreal) getYPixCanvas(_topMargin / 2.0);
  
  QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 4);
    
  // draw the text
    
  painter.drawText(bRect, Qt::AlignTop, title.c_str());

}

/////////////////////////////////////////////////////////	
// Y axis label
    
void WorldPlot::drawYAxisLabelLeft(QPainter &painter,
                                   const string &label) 

{

  // get bounding rectangle
  
  QRect tRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    
  // qreal xx = (qreal) (getXPixCanvas(tRect.height() / 2.0));
  qreal yy = (qreal) ((_yMinPixel + _yMaxPixel + tRect.width()) / 2.0);

  qreal xx =
    (qreal) (_xMinPixel - tRect.height() - _axisTextMargin);
  if (_yAxisLabelsInside) {
    xx = (qreal) (_xMinPixel + _axisTextMargin);
  }

  QRectF bRect(0, 0, tRect.width() + 2, tRect.height() + 2);

  painter.save();
  painter.translate(xx, yy);
  painter.rotate(-90);
  painter.drawText(bRect, Qt::AlignCenter, label.c_str());
  painter.restore();
	
}

///////////////////////	
// Legends at top left
    
void WorldPlot::drawLegendsTopLeft(QPainter &painter,
                                   const vector<string> &legends) 

{

  qreal xx = (qreal) (_xMinPixel + _yAxisTickLen + _legendTextMargin);
  qreal yy = _yMaxPixel + _xAxisTickLen;
  
  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy += (_legendTextMargin + tRect.height());
  }

}

///////////////////////	
// Legends at top right
    
void WorldPlot::drawLegendsTopRight(QPainter &painter,
                                    const vector<string> &legends)

{

  qreal yy = _yMaxPixel + _yAxisTickLen;
  
  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    qreal xx = (qreal) (_xMaxPixel - _yAxisTickLen -
                        _legendTextMargin - tRect.width());
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy += (_legendTextMargin + tRect.height());
  }

}

//////////////////////////	
// Legends at bottom left
    
void WorldPlot::drawLegendsBottomLeft(QPainter &painter,
                                      const vector<string> &legends) 

{
	
  qreal xx = (qreal) (_xMinPixel + _yAxisTickLen + _legendTextMargin);
  qreal yy = (qreal) (_yMinPixel - _xAxisTickLen);

  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    yy -= tRect.height();
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy -= _legendTextMargin;
  }

}

///////////////////////////	
// Legends at bottom right
    
void WorldPlot::drawLegendsBottomRight(QPainter &painter,
                                       const vector<string> &legends) 

{
	
  qreal yy = (qreal) (_yMinPixel - _xAxisTickLen);

  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    qreal xx = (qreal) (_xMaxPixel - _yAxisTickLen -
                        _legendTextMargin - tRect.width());
    yy -= tRect.height();
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy -= _legendTextMargin;
  }

}

/////////////	
// left axis
    
void WorldPlot::drawAxisLeft(QPainter &painter,
                             const string &units,
                             bool doLine,
                             bool doTicks,
                             bool doLabels,
                             bool doGrid) 
  
{

  // axis line

  if (doLine) {
    drawLine(painter, _xMinWorld, _yMinWorld, _xMinWorld, _yMaxWorld);
  }

  // font

  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  painter.setFont(font);

  // axis units label

  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX =
    (qreal) (_xMinPixel - unitsRect.width() - _axisTextMargin);
  if (_yAxisLabelsInside) {
    unitsX = (qreal) (_xMinPixel + _axisTextMargin);
  }
  qreal unitsY = (qreal) (_yMaxPixel + (unitsRect.height() / 2));
  QRectF bRect(unitsX, unitsY, 
               unitsRect.width() + 2, unitsRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _leftTicks = linearTicks(_yMinWorld, _yMaxWorld,
                           _yNTicksIdeal, _ySpecifyTicks,
                           _yTickMin, _yTickDelta);

  if (_leftTicks.size() < 2) {
    return;
  }
	
  double delta = _leftTicks[1] - _leftTicks[0];
  for (size_t i = 0; i < _leftTicks.size(); i++) {
	    
    double val = _leftTicks[i];
    double ypix = getYPixel(val);
    if (doTicks) {
      QLineF qline(_xMinPixel, ypix,
                   _xMinPixel + _yAxisTickLen, ypix);
      painter.drawLine(qline);
    }
	    
    // grid
    
    if (doGrid) {
      if (ypix != _yMinPixel && ypix != _yMaxPixel) {
        painter.save();
        painter.setPen(_gridColor.c_str());
        QLineF qlineGrid(_xMinPixel, ypix,
                         _xMaxPixel, ypix);
        painter.drawLine(qlineGrid);
        painter.restore();
      }
    }

    // labels

    font.setPointSizeF(_tickValuesFontSize);
    painter.setFont(font);

    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    qreal labelX = (qreal) (_xMinPixel - labelRect.width() - _axisTextMargin);
    if (_yAxisLabelsInside) {
      labelX = (qreal) (_xMinPixel + _axisTextMargin);
    }
    qreal labelY = (qreal) (ypix - (labelRect.height() / 2));
    QRectF bRect2(labelX, labelY,
                  labelRect.width() + 2, labelRect.height() + 2);
    if ((fabs(labelY - unitsY) > labelRect.height() + _axisTextMargin) &&
        (fabs(labelY - _yMinPixel) > labelRect.height() + _axisTextMargin)) {
      if (doLabels) {
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }

  } // i
	
}

//////////////
// right axis
  
void WorldPlot::drawAxisRight(QPainter &painter,
                              const string &units,
                              bool doLine,
                              bool doTicks,
                              bool doLabels,
                              bool doGrid) 

{
	
  // axis line

  if (doLine) {
    drawLine(painter, _xMaxWorld, _yMinWorld, _xMaxWorld, _yMaxWorld);
  }

  // font

  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  painter.setFont(font);

  // axis units label
	
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel + _axisTextMargin);
  qreal unitsY = (qreal) (_yMaxPixel + (unitsRect.height() / 2));
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _rightTicks = linearTicks(_yMinWorld, _yMaxWorld,
                            _yNTicksIdeal, _ySpecifyTicks,
                            _yTickMin, _yTickDelta);
  if (_rightTicks.size() < 2) {
    return;
  }
	
  double delta = _rightTicks[1] - _rightTicks[0];
  for (size_t i = 0; i < _rightTicks.size(); i++) {
	    
    double val = _rightTicks[i];
    double ypix = getYPixel(val);
    if (doTicks) {
      QLineF qline(_xMaxPixel, ypix,
                   _xMaxPixel - _yAxisTickLen, ypix);
      painter.drawLine(qline);
    }
	    
    // grid
    
    if (doGrid) {
      if (ypix != _yMinPixel && ypix != _yMaxPixel) {
        painter.save();
        painter.setPen(_gridColor.c_str());
        QLineF qlineGrid(_xMinPixel, ypix,
                         _xMaxPixel, ypix);
        painter.drawLine(qlineGrid);
        painter.restore();
      }
    }

    // labels

    font.setPointSizeF(_tickValuesFontSize);
    painter.setFont(font);

    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    qreal labelX = (qreal) (_xMaxPixel +_axisTextMargin);
    qreal labelY = (qreal) (ypix - (labelRect.height() / 2));
    QRectF bRect2(labelX, labelY,
                  labelRect.width() + 2, labelRect.height() + 2);

    if ((fabs(labelY - unitsY) > labelRect.height() + _axisTextMargin) &&
        (fabs(labelY - _yMinPixel) > labelRect.height() + _axisTextMargin)) {
      if (doLabels) {
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }
      
  }
	
} // drawAxisRight

///////////////  
// bottom axis
    
void WorldPlot::drawAxisBottom(QPainter &painter,
                               const string &units,
                               bool doLine,
                               bool doTicks,
                               bool doLabels,
                               bool doGrid) 

{

  // axis line
  
  if (doLine) {
    drawLine(painter, _xMinWorld, _yMinWorld, _xMaxWorld, _yMinWorld);
  }
    
  // font

  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  painter.setFont(font);

  // axis units label
	
  QRect capRect(painter.fontMetrics().tightBoundingRect("XXX"));
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel - unitsRect.width());
  qreal unitsY = (qreal) (_yMinPixel + capRect.height() - 2);
  if (_xAxisLabelsInside) {
    unitsY = (qreal) (_yMinPixel - capRect.height() - 2);
  }
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, capRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _bottomTicks = linearTicks(_xMinWorld, _xMaxWorld,
                             _xNTicksIdeal, _xSpecifyTicks,
                             _xTickMin, _xTickDelta);
  if (_bottomTicks.size() < 2) {
    return;
  }
	
  double delta = _bottomTicks[1] - _bottomTicks[0];
  for (size_t i = 0; i < _bottomTicks.size(); i++) {

    // ticks

    double val = _bottomTicks[i];
    double xpix = getXPixel(val);
    if (doTicks) {
      QLineF qline(xpix, _yMinPixel,
                   xpix, _yMinPixel - _xAxisTickLen);
      painter.drawLine(qline);
    }
	    
    // grid
    
    if (doGrid) {
      if (xpix != _xMinPixel && xpix != _xMaxPixel) {
        painter.save();
        painter.setPen(_gridColor.c_str());
        QLineF qlineGrid(xpix, _yMinPixel,
                         xpix, _yMaxPixel);
        painter.drawLine(qlineGrid);
        painter.restore();
      }
    }
    
    // labels

    font.setPointSizeF(_tickValuesFontSize);
    painter.setFont(font);

    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    if (((xpix + labelRect.width() / 2 + _axisTextMargin) < unitsX) &&
        ((xpix - labelRect.width() / 2 - _axisTextMargin) > _xMinPixel)) {
      qreal labelX = (qreal) (xpix - labelRect.width() / 2.0);
      qreal labelY = unitsY;
      QRectF bRect2(labelX, labelY,
                    labelRect.width() + 2, labelRect.height() + 2);
      if (doLabels) {
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }
      
  }
	
} // drawAxisBottom

///////////  
// top axis

void WorldPlot::drawAxisTop(QPainter &painter,
                            const string &units,
                            bool doLine,
                            bool doTicks,
                            bool doLabels,
                            bool doGrid)

{
	
  // axis line
	
  if (doLine) {
    drawLine(painter, _xMinWorld, _yMaxWorld, _xMaxWorld, _yMaxWorld);
  }
	
  // font

  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  painter.setFont(font);

  // axis units label
	
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel - unitsRect.width() / 2);
  qreal unitsY = (qreal) (_yMaxPixel - (unitsRect.height() + _axisTextMargin) - 2);
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _topTicks = linearTicks(_xMinWorld, _xMaxWorld,
                          _xNTicksIdeal, _xSpecifyTicks,
                          _xTickMin, _xTickDelta);
  if (_topTicks.size() < 2) {
    return;
  }
  
  double delta = _topTicks[1] - _topTicks[0];
  for (size_t i = 0; i < _topTicks.size(); i++) {
	    
    double val = _topTicks[i];
    double xpix = getXPixel(val);
    if (doTicks) {
      QLineF qline(xpix, _yMaxPixel,
                   xpix, _yMaxPixel + _xAxisTickLen);
      painter.drawLine(qline);
    }
    
    // grid
    
    if (doGrid) {
      if (xpix != _xMinPixel && xpix != _xMaxPixel) {
        painter.setPen(_gridColor.c_str());
        QLineF qlineGrid(xpix, _yMinPixel,
                         xpix, _yMaxPixel);
        painter.drawLine(qlineGrid);
      }
    }

    // labels

    font.setPointSizeF(_tickValuesFontSize);
    painter.setFont(font);

    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().boundingRect(label.c_str()));
    if (((xpix + labelRect.width() / 2 + _axisTextMargin) < unitsX) &&
        ((xpix - labelRect.width() / 2 - _axisTextMargin) > _xMinPixel)) {
      qreal labelX = (qreal) (xpix - labelRect.width() / 2.0);
      qreal labelY = unitsY - 2;
      QRectF bRect2(labelX, labelY,
                    labelRect.width() + 2, labelRect.height() + 2);
      if (doLabels) {
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }
      
  }
	
} // drawAxisTop

/////////////////////////
// draw axes bounding box
    
void WorldPlot::drawAxesBox(QPainter &painter)
  
{
  
  drawLine(painter, _xMinWorld, _yMinWorld, _xMinWorld, _yMaxWorld);
  drawLine(painter, _xMaxWorld, _yMinWorld, _xMaxWorld, _yMaxWorld);
  drawLine(painter, _xMinWorld, _yMinWorld, _xMaxWorld, _yMinWorld);
  drawLine(painter, _xMinWorld, _yMaxWorld, _xMaxWorld, _yMaxWorld);

}

//////////////////////////////////////////////////////  
// compute linear ticks for normal axis
  
vector<double> WorldPlot::linearTicks(double minVal,
                                      double maxVal,
                                      int nTicksIdeal,
                                      bool specifyTicks,
                                      double specifiedTickMin,
                                      double specifiedTickDelta) 
  
{

  // are ticks specified?

  if (specifyTicks) {
    vector<double> ticks;
    double tickVal = specifiedTickMin;
    while (tickVal <= maxVal) {
      if (tickVal >= minVal && tickVal <= maxVal) {
        ticks.push_back(tickVal);
      }
      tickVal += specifiedTickDelta;
    }
    return ticks;
  }

  bool reversed = false;
  if (minVal > maxVal) {
    reversed = true;
    double tmp = minVal;
    minVal = maxVal;
    maxVal = tmp;
  }

  double  approxInterval =
    (maxVal - minVal) / (double) (nTicksIdeal + 1);
  double logInterval = log10(fabs(approxInterval));
  double intPart;
  double fractPart = modf(logInterval, &intPart);

  if (fractPart < 0) {
    fractPart += 1.0;
    intPart -= 1.0;
  }
	
  double rem = pow(10.0, fractPart);
  double base;
  if (rem > 7.5) {
    base = 10.0;
  } else if (rem > 3.5) {
    base = 5.0;
  } else if (rem > 1.5) {
    base = 2.0;
  } else {
    base = 1.0;
  }
	
  double deltaTick = (base * pow (10.0, intPart));
  double tickMin = floor(minVal / deltaTick) * deltaTick;
  if (tickMin < minVal) {
    tickMin += deltaTick;
  }
  int nTicks = (int) ((maxVal - tickMin) / deltaTick) + 1;
  if (nTicks < 2) {
    nTicks = 2;
  }
	
  vector<double> ticks;
  if (reversed) {
    for (int i = 0; i < nTicks; i++) {
      ticks.push_back(tickMin + (nTicks - i - 1) * deltaTick);
    }
  } else {
    for (int i = 0; i < nTicks; i++) {
      ticks.push_back(tickMin + i * deltaTick);
    }
  }

  return ticks;
	
}

////////////////////////////////
// range axes

void WorldPlot::drawRangeAxes(QPainter &painter,
                              const string &units,
                              bool drawGrid,
                              const QColor &lineColor,
                              const QColor &gridColor,
                              const QColor &textColor,
                              const QFont &labelFont,
                              const QFont &valuesFont,
                              bool unitsInFeet)
                              
{

  double unitsMult = 1.0;
  if (unitsInFeet) {
    unitsMult = 1.0 / 0.3048;
  }

  // save the painter state

  painter.save();

  // axis lines

  painter.setPen(lineColor);
  drawLine(painter, _xMinWorld, _yMinWorld, _xMinWorld, _yMaxWorld);
  drawLine(painter, _xMaxWorld, _yMinWorld, _xMaxWorld, _yMaxWorld);

  // units label

  painter.setPen(textColor);
  painter.setFont(labelFont);
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsXLeft =
    (qreal) (_xMinPixel - unitsRect.width() - _axisTextMargin);
  qreal unitsY = (qreal) (_yMaxPixel + (unitsRect.height() / 2));
  QRectF bRectLeft(unitsXLeft, unitsY, 
                   unitsRect.width() + 2, unitsRect.height() + 2);
  painter.drawText(bRectLeft, Qt::AlignCenter, units.c_str());

  qreal unitsXRight = (qreal) (_xMaxPixel + _axisTextMargin);
  QRectF bRectRight(unitsXRight, unitsY,
                    unitsRect.width() + 2, unitsRect.height() + 2);
  painter.drawText(bRectRight, Qt::AlignCenter, units.c_str());

  // tick marks

  vector<double> ticks = linearTicks(_yMinWorld * unitsMult,
                                     _yMaxWorld * unitsMult,
                                     _yNTicksIdeal, _ySpecifyTicks,
                                     _yTickMin, _yTickDelta);
  if (ticks.size() < 1) {
    painter.restore();
    return;
  }

  double delta = ticks[0];
  if (ticks.size() > 1) {
    delta = ticks[1] - ticks[0];
  }

  for (size_t i = 0; i < ticks.size(); i++) {

    painter.setPen(lineColor);

    // ticks left

    double val = ticks[i];
    double pix = getYPixel(val / unitsMult);
    QLineF qlineLeft(_xMinPixel, pix,
                     _xMinPixel + _yAxisTickLen, pix);
    painter.drawLine(qlineLeft);
    
    // ticks right
    
    QLineF qlineRight(_xMaxPixel, pix,
                      _xMaxPixel - _yAxisTickLen, pix);
    painter.drawLine(qlineRight);

    // grid?
    
    if (drawGrid) {
      if (pix != _yMinPixel && pix != _yMaxPixel) {
        painter.setPen(gridColor);
        QLineF qlineGrid(_xMinPixel, pix,
                         _xMaxPixel, pix);
        painter.drawLine(qlineGrid);
      }
    }
    
    // value labels

    painter.setPen(textColor);
    painter.setFont(valuesFont);
    
    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    qreal labelXLeft = (qreal) (_xMinPixel - labelRect.width() - _axisTextMargin);
    qreal labelXRight = (qreal) (_xMaxPixel + _axisTextMargin);
    qreal labelY = (qreal) (pix - (labelRect.height() / 2));
    QRectF bRect2Left(labelXLeft, labelY,
                      labelRect.width() + 2, labelRect.height() + 2);
    QRectF bRect2Right(labelXRight, labelY,
                       labelRect.width() + 2, labelRect.height() + 2);
    if ((fabs(labelY - unitsY) > labelRect.height() + _axisTextMargin) &&
        (fabs(labelY - _yMinPixel) > labelRect.height() + _axisTextMargin)) {
      painter.drawText(bRect2Left, Qt::AlignCenter, label.c_str());
      painter.drawText(bRect2Right, Qt::AlignCenter, label.c_str());
    }

  }
	
  // restore the painter state

  painter.restore();

}

	
////////////////////////////////
// time axes
    
void WorldPlot::drawTimeAxes(QPainter &painter,
                             const RadxTime &startTime,
                             const RadxTime &endTime,
                             bool drawGrid,
                             const QColor &lineColor,
                             const QColor &gridColor,
                             const QColor &textColor,
                             const QFont &labelFont,
                             const QFont &valuesFont,
                             bool drawDistTicks)

{
	
  // save the painter state
  
  painter.save();
  
  // axis lines
  
  painter.setPen(lineColor);

  drawLine(painter, _xMinWorld, _yMinWorld, _xMaxWorld, _yMinWorld);
  drawLine(painter, _xMinWorld, _yMaxWorld, _xMaxWorld, _yMaxWorld);

  // get size of typical time string
  
  string templStr("00:00:00");
  QRect templRect(painter.fontMetrics().tightBoundingRect(templStr.c_str()));
  double minIntervalSecs = (templRect.width() / _xPixelsPerWorld) * 3.0;
  int intervalSecs = 86400;

  int canonicalSecs[] = { 1, 2, 3, 5, 10, 15, 20, 30,
                          1 * 60, 2 * 60, 3 * 60, 5 * 60,
                          10 * 60, 15 * 60, 20 * 60, 30 * 60,
                          1 * 3600, 2 * 3600, 3 * 3600, 4 * 3600,
                          6 * 3600, 8 * 3600, 12 * 3600, 24 * 3600 };

  int nCanonical = sizeof(canonicalSecs) / sizeof(int);

  for (int ii = nCanonical - 1; ii > 0; ii--) {
    if (minIntervalSecs >= canonicalSecs[ii - 1]) {
      intervalSecs = canonicalSecs[ii];
      break;
    }
  }

  time_t startUsecs =
    ((time_t) (startTime.utime() / intervalSecs) + 1) * intervalSecs;
  time_t endUsecs =
    ((time_t) (endTime.utime() / intervalSecs)) * intervalSecs;
  int nTicks = (endUsecs - startUsecs) / intervalSecs + 1;
  
  vector<RadxTime> ticks;
  for (int ii = 0; ii < nTicks; ii++) {
    RadxTime tickTime(startUsecs + ii * intervalSecs);
    ticks.push_back(tickTime);
  }

  // cmopute height of labels

  painter.setFont(valuesFont);
  QRect testRect(painter.fontMetrics().tightBoundingRect("test"));
  int labelHt = testRect.height();

  // axis units label
  
  painter.setPen(textColor);
  painter.setFont(labelFont);
  
  string units("UTC");
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel - unitsRect.width() / 2);
  qreal unitsY =
    (qreal) (_yMinPixel + (unitsRect.height() + _axisTextMargin));
  if (drawDistTicks) {
    unitsY += (int) (labelHt * 3.0 / 2.0 + 0.5);
  }
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 2);
  painter.drawText(bRect, Qt::AlignCenter, units.c_str());

  // tick marks

  for (size_t i = 0; i < ticks.size(); i++) {
    
    painter.setPen(lineColor);

    const RadxTime &tickTime = ticks[i];
    double val = tickTime - startTime;
    double pix = getXPixel(val);
    QLineF qlineBottom(pix, _yMinPixel, pix, _yMinPixel - _xAxisTickLen);
    painter.drawLine(qlineBottom);
    QLineF qlineTop(pix, _yMaxPixel, pix, _yMaxPixel + _xAxisTickLen);
    painter.drawLine(qlineTop);

    // time labels - bottom axis
    
    painter.setPen(textColor);
    painter.setFont(valuesFont);

    char timeLabel[1024];
    sprintf(timeLabel, "%.2d:%.2d:%.2d",
            tickTime.getHour(), tickTime.getMin(), tickTime.getSec());
    QRect labelRect(painter.fontMetrics().tightBoundingRect(timeLabel));
    if (((pix + labelRect.width() / 2 + _axisTextMargin) < unitsX) &&
        ((pix - labelRect.width() / 2 - _axisTextMargin) > _xMinPixel)) {
      qreal labelX = (qreal) (pix - labelRect.width() / 2.0);
      qreal labelY = unitsY;
      QRectF bRect2(labelX, labelY,
                    labelRect.width() + 2, labelRect.height() + 2);
      painter.drawText(bRect2, Qt::AlignCenter, timeLabel);
    }

    // draw a vertical line for day divider
    
    if (tickTime.getHour() == 0 && 
        tickTime.getMin() == 0 && 
        tickTime.getSec() == 0) {
      painter.setPen(lineColor);
      QLineF qlineDivider(pix, _yMinPixel, pix, _yMaxPixel);
      painter.drawLine(qlineDivider);
    }

    if (drawGrid) {
      if (pix != _xMinPixel && pix != _xMaxPixel) {
        painter.setPen(gridColor);
        QLineF qlineDivider(pix, _yMinPixel, pix, _yMaxPixel);
        painter.drawLine(qlineDivider);
      }
    }

  }
	
  // date labels - top axis

  painter.setPen(textColor);
  painter.setFont(valuesFont);

  string startTimeLabel = startTime.asString(3);
  string endTimeLabel = endTime.asString(3);
  
  QRect startRect
    (painter.fontMetrics().tightBoundingRect(startTimeLabel.c_str()));
  qreal startX = _xMinPixel;
  qreal startY = _yMaxPixel - startRect.height() * 2;
  QRectF startRect2(startX, startY, startRect.width() + 2, 
                    startRect.height() + 2);
  painter.drawText(startRect2, Qt::AlignCenter, startTimeLabel.c_str());
  
  QRect 
    endRect(painter.fontMetrics().tightBoundingRect(endTimeLabel.c_str()));
  qreal endX = _xMaxPixel - endRect.width();
  qreal endY = startY;
  QRectF endRect2(endX, endY, endRect.width() + 2, endRect.height() + 2);
  painter.drawText(endRect2, Qt::AlignCenter, endTimeLabel.c_str());

  // restore painter to original state

  painter.restore();
      
} // drawTimeAxes

////////////////////////////////
// distance ticks on time axis

void WorldPlot::drawDistanceTicks(QPainter &painter,
                                  const RadxTime &startTime,
                                  const vector<double> &tickDists,
                                  const vector<RadxTime> &tickTimes,
                                  const QColor &lineColor,
                                  const QColor &textColor,
                                  const QFont &valuesFont)
  
{
	
  // save the painter state
  
  painter.save();
  
  // loop through ticks, ignoring the first one

  double prevPix = -9999;

  for (size_t i = 0; i < tickDists.size(); i++) {
    
    const RadxTime &tickTime = tickTimes[i];
    double distVal = tickDists[i];
    
    if (!std::isfinite(distVal)) {
      continue;
    }

    double timeVal = tickTime - startTime;
    double pix = getXPixel(timeVal);
    double deltaPix = pix - prevPix;
    prevPix = pix;

    // label

    painter.setPen(textColor);
    painter.setFont(valuesFont);
    
    char distLabel[1024];
    sprintf(distLabel, "%gkm", distVal);
    QRect labelRect(painter.fontMetrics().tightBoundingRect(distLabel));
    
    if (fabs(deltaPix) < labelRect.width() + 2) {
      continue;
    }

    // qreal maxX = (qreal) (_xMaxPixel - labelRect.width() / 2);

    // if (((pix + labelRect.width() / 2 + 2) < maxX) &&
    //     ((pix - labelRect.width() / 2 - 2) > _xMinPixel)) {

      qreal labelX = (qreal) (pix - labelRect.width() / 2.0);
      qreal labelY =
        (qreal) (_yMinPixel + labelRect.height());

      QRectF bRect2(labelX, labelY,
                    labelRect.width() + 2, labelRect.height() + 2);
      painter.drawText(bRect2, Qt::AlignCenter, distLabel);

      // ticks above bottom axis
      
      painter.setPen(lineColor);
      
      QLineF qline(pix, _yMinPixel, pix, _yMinPixel - _xAxisTickLen);
      painter.drawLine(qline);
      
    // }

  } //  i
	
  // restore painter to original state

  painter.restore();
      
} // drawTimeAxes

///////////////////////////////////////////////////////////////
// draw source image into graphics, scaling and translating to
// map the world coordinates
    
void WorldPlot::drawImage(QPainter &painter, QImage &image,
                          double xMinWorldImage, double xMaxWorldImage,
                          double yMinWorldImage, double yMaxWorldImage) 
{
	
  qreal xMinPixelDest = getXPixel(xMinWorldImage);
  qreal xMaxPixelDest = getXPixel(xMaxWorldImage);
    
  qreal yMinPixelDest = getYPixel(yMinWorldImage);
  qreal yMaxPixelDest = getYPixel(yMaxWorldImage);

  double destWidth = xMaxPixelDest - xMinPixelDest;
  double destHeight = yMinPixelDest - yMaxPixelDest;

  QRectF destRect(xMinPixelDest, yMaxPixelDest, destWidth, destHeight);

  setClippingOn(painter);
  painter.drawImage(destRect, image);
  setClippingOff(painter);

}

///////////////////////////////////////
// set clipping on to between margins

void WorldPlot::setClippingOn(QPainter &painter) {
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);
  painter.setClipping(true);
}

///////////////////  
// set clipping off

void WorldPlot::setClippingOff(QPainter &painter) {
  painter.setClipping(false);
}

////////////////////////////////////////////////////
// get axis label given value and delta
    
string WorldPlot::getAxisLabel(double delta, double val)

{

  char text[1024];
  delta = fabs(delta);
  
  if (delta >= 1.0) {
    sprintf(text, "%.0f", val);
  } else if (delta >= 0.1) {
    sprintf(text, "%.1f", val);
  } else if (delta >= 0.01) {
    sprintf(text, "%.2f", val);
  } else if (delta >= 0.001) {
    sprintf(text, "%.3f", val);
  } else if (delta >= 0.0001) {
    sprintf(text, "%.4f", val);
  } else {
    sprintf(text, "%.5f", val);
  }

  return text;
    
}

///////////////////////////////////////
// compute transform from basic values
                              
void WorldPlot::_computeTransform() 
{
    
  _plotWidth = _widthPixels - _leftMargin - _rightMargin - _colorScaleWidth;
  _plotHeight = _heightPixels - _topMargin - _bottomMargin;
    
  _xMinPixel = _leftMargin + _xPixOffset;
  _xMaxPixel = _xMinPixel + _plotWidth - 1;
  _yMaxPixel = _topMargin + _yPixOffset;
  _yMinPixel = _yMaxPixel + _plotHeight - 1;
    
  _xPixelsPerWorld =
    (_xMaxPixel - _xMinPixel) / (_xMaxWorld - _xMinWorld);
  _yPixelsPerWorld =
    (_yMaxPixel - _yMinPixel) / (_yMaxWorld - _yMinWorld);
    
  _transform.reset();
  _transform.translate(_xMinPixel, _yMinPixel);
  _transform.scale(_xPixelsPerWorld, _yPixelsPerWorld);
  _transform.translate(-_xMinWorld, -_yMinWorld);
    
  _xMinWindow = getXWorld(_xPixOffset);
  _yMinWindow = getYWorld(_yPixOffset);
  _xMaxWindow = getXWorld(_xPixOffset + _widthPixels);
  _yMaxWindow = getYWorld(_yPixOffset + _heightPixels);

}

/////////////////////////////////////////////////////
// draw the color scale

void WorldPlot::drawColorScale(const ColorMap &colorMap,
                               QPainter &painter,
                               int unitsFontSize)
  
{

  const std::vector<ColorMap::CmapEntry> &cmap = colorMap.getEntries();

  int pltHt = _plotHeight - _topMargin;
  int width = _colorScaleWidth;
  int xStart = _widthPixels - width;
  size_t nHts = cmap.size() + 1; // leave some space at top and bottom
  double patchHt = (double) (pltHt) / (double) nHts;
  int iPatchHt = (int) patchHt;
  int yStart = _topMargin + iPatchHt / 2;

  // clear rectangle
  
  QRectF cRect((qreal) xStart, (qreal) 0, _colorScaleWidth, _plotHeight);
  QColor backColor(_backgroundColor.c_str());
  painter.fillRect(cRect, backColor);
                      
  // fill the swatches with the color
  
  painter.save();
  painter.setPen(Qt::SolidLine);
  int scaleYTop = 0, scaleYBot = 0;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    QColor color(entry.red, entry.green, entry.blue);
    painter.setBrush(color);
    double topY = yStart + pltHt - (int) (ii + 2) * patchHt + (patchHt / 2);
    QRectF r(xStart, topY, width, patchHt);
    painter.fillRect(r, color);
    if (ii == 0) {
      scaleYBot = topY + patchHt;
    } else if (ii == cmap.size() - 1) {
      scaleYTop = topY;
    }
  }
  painter.restore();
  
  // get precision based on data
  
  double minDelta = 1.0e99;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    double delta = fabs(entry.maxVal - entry.minVal);
    if (delta < minDelta) minDelta = delta;
  }
  int ndecimals = 0;
  char format = 'f';
  if (minDelta <= 0.025) {
    ndecimals = 3;
    format = 'g';
  } else if (minDelta <= 0.05) {
    ndecimals = 3;
  } else if (minDelta <= 0.25) {
    ndecimals = 2;
  } else if (minDelta <= 25) {
    ndecimals = 1;
  }

  // save state

  painter.save();

  // scale the font
  
  QFont defaultFont = painter.font();
  if (defaultFont.pointSize() > patchHt / 3) {
    int pointSize = patchHt / 3;
    if (pointSize < 7) {
      pointSize = 7;
    }
    QFont scaledFont(defaultFont);
    scaledFont.setPointSizeF(pointSize);
    painter.setFont(scaledFont);
  }
  
  // add labels

  painter.setBrush(Qt::black);
  painter.setBackgroundMode(Qt::OpaqueMode);
  QRect tRect(painter.fontMetrics().tightBoundingRect("1.0"));
  int textHt = tRect.height();
  
  if (colorMap.labelsSetByValue()) {
  
    // label values specified in the color scale file

    const vector<ColorMap::CmapLabel> &labels = colorMap.getSpecifiedLabels();
    double scaleHeight = scaleYBot - scaleYTop;
    for (size_t ii = 0; ii < labels.size(); ii++) {
      const ColorMap::CmapLabel &label = labels[ii];
      double yy = scaleYBot - scaleHeight * label.position;
      painter.drawText(xStart, (int) yy - textHt / 2, 
                       width + 4, textHt + 4, 
                       Qt::AlignCenter | Qt::AlignHCenter, 
                       label.text.c_str());
    } // ii

  } else {

    // label the color transitions
    // we space the labels vertically by at least 2 * text height
    
    double yy = yStart + pltHt - (patchHt * 1.0);
    double prevIyy = -1;
    for (size_t ii = 0; ii < cmap.size(); ii++) {
      const ColorMap::CmapEntry &entry = cmap[ii];
      QString label = QString("%1").arg(entry.minVal,0,format,ndecimals);
      int iyy = (int) yy;
      bool doText = false;
      if (prevIyy < 0) {
        doText = true;
      } else if ((prevIyy - iyy) > textHt * 2) {
        doText = true;
      }
      if (doText) {
        painter.drawText(xStart, iyy, width, iPatchHt, 
                         Qt::AlignCenter | Qt::AlignHCenter, 
                         label);
        prevIyy = iyy;
      }
      yy -= patchHt;
    }
    
    // last label at top
    
    const ColorMap::CmapEntry &entry = cmap[cmap.size()-1];
    QString label = QString("%1").arg(entry.maxVal,0,format,ndecimals);
    int iyy = (int) yy;
    if ((prevIyy - iyy) > textHt * 2) {
      painter.drawText(xStart, iyy, width, iPatchHt, 
                       Qt::AlignVCenter | Qt::AlignHCenter, 
                       label);
    }
    yy -= patchHt;

  }

  // add units label

  string units(colorMap.getUnits());
  if (units.size() > 0) {
    
    QFont ufont(painter.font());
    ufont.setPointSizeF(unitsFontSize);
    painter.setFont(ufont);

    QRect tRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
    int iyy = _topMargin / 2;
    int ixx = _widthPixels - width;
    QString qunits(units.c_str());
    painter.drawText(ixx, iyy, width, tRect.height() + 4, 
                     Qt::AlignTop | Qt::AlignHCenter, qunits);
    
  }

  // restore state
  
  painter.restore();

}

///////////////////////
// represent as string
  
string WorldPlot::asString() {
    
  stringstream sstr;

  sstr << "_widthPixels: " << _widthPixels << "\n"
       << "_heightPixels: " << _heightPixels << "\n"
       << "_leftMargin: " << _leftMargin << "\n"
       << "_rightMargin: " << _rightMargin << "\n"
       << "_topMargin: " << _topMargin << "\n"
       << "_bottomMargin: " << _bottomMargin << "\n"
       << "_xMinWorld: " << _xMinWorld << "\n"
       << "_xMaxWorld: " << _xMaxWorld << "\n"
       << "_yMinWorld: " << _yMinWorld << "\n"
       << "_yMaxWorld: " << _yMaxWorld << "\n"
       << "_plotWidth: " << _plotWidth << "\n"
       << "_plotHeight: " << _plotHeight << "\n"
       << "_xMinPixel: " << _xMinPixel << "\n"
       << "_xMaxPixel: " << _xMaxPixel << "\n"
       << "_yMinPixel: " << _yMinPixel << "\n"
       << "_yMaxPixel: " << _yMaxPixel << "\n"
       << "_xPixelsPerWorld: " << _xPixelsPerWorld << "\n"
       << "_yPixelsPerWorld: " << _yPixelsPerWorld << "\n"
       << "_xMinWindow: " << _xMinWindow << "\n"
       << "_xMaxWindow: " << _xMaxWindow << "\n"
       << "_yMinWindow: " << _yMinWindow << "\n"
       << "_yMaxWindow: " << _yMaxWindow << "\n"
       << "";

  return sstr.str();
	
}

/////////////////////////////////////////////////////
// print

void WorldPlot::print(ostream &out)
  
{

  out << "================= WorldPlot properties ===================" << endl;

  out << "  _widthPixels       : " << _widthPixels << endl;
  out << "  _heightPixels      : " << _heightPixels << endl;
  out << "  _xPixOffset        : " << _xPixOffset << endl;
  out << "  _yPixOffset        : " << _yPixOffset << endl;
  out << "  _leftMargin        : " << _leftMargin << endl;
  out << "  _rightMargin       : " << _rightMargin << endl;
  out << "  _topMargin         : " << _topMargin << endl;
  out << "  _bottomMargin      : " << _bottomMargin << endl;
  out << "  _axisTextMargin    : " << _axisTextMargin << endl;
  out << "  _legendTextMargin  : " << _legendTextMargin << endl;
  out << "  _colorScaleWidth   : " << _colorScaleWidth << endl;
  out << "  _xAxisTickLen      : " << _xAxisTickLen << endl;
  out << "  _xNTicksIdeal      : " << _xNTicksIdeal << endl;
  out << "  _xSpecifyTicks     : " << _xSpecifyTicks << endl;
  out << "  _xTickMin          : " << _xTickMin << endl;
  out << "  _xTickDelta        : " << _xTickDelta << endl;
  out << "  _yAxisTickLen      : " << _yAxisTickLen << endl;
  out << "  _yNTicksIdeal      : " << _yNTicksIdeal << endl;
  out << "  _ySpecifyTicks     : " << _ySpecifyTicks << endl;
  out << "  _yTickMin          : " << _yTickMin << endl;
  out << "  _yTickDelta        : " << _yTickDelta << endl;
  out << "  _xAxisLabelsInside : " << _xAxisLabelsInside << endl;
  out << "  _yAxisLabelsInside : " << _yAxisLabelsInside << endl;
  out << "  _titleFontSize     : " << _titleFontSize << endl;
  out << "  _axisLabelFontSize : " << _axisLabelFontSize << endl;
  out << "  _tickValuesFontSize: " << _tickValuesFontSize << endl;
  out << "  _legendFontSize    : " << _legendFontSize << endl;
  out << "  _titleColor        : " << _titleColor << endl;
  out << "  _axisLineColor     : " << _axisLineColor << endl;
  out << "  _axisTextColor     : " << _axisTextColor << endl;
  out << "  _gridColor         : " << _gridColor << endl;
  out << "  _xMinWorld         : " << _xMinWorld << endl;
  out << "  _xMaxWorld         : " << _xMaxWorld << endl;
  out << "  _yMinWorld         : " << _yMinWorld << endl;
  out << "  _yMaxWorld         : " << _yMaxWorld << endl;
  out << "  _plotWidth         : " << _plotWidth << endl;
  out << "  _plotHeight        : " << _plotHeight << endl;
  out << "  _xMinPixel         : " << _xMinPixel << endl;
  out << "  _yMinPixel         : " << _yMinPixel << endl;
  out << "  _xMaxPixel         : " << _xMaxPixel << endl;
  out << "  _yMaxPixel         : " << _yMaxPixel << endl;
  out << "  _xPixelsPerWorld   : " << _xPixelsPerWorld << endl;
  out << "  _yPixelsPerWorld   : " << _yPixelsPerWorld << endl;
  out << "  _xMinWindow        : " << _xMinWindow << endl;
  out << "  _xMaxWindow        : " << _xMaxWindow << endl;
  out << "  _yMinWindow        : " << _yMinWindow << endl;
  out << "  _yMaxWindow        : " << _yMaxWindow << endl;

  if (_topTicks.size() > 0) {
    out << "  _topTicks          : ";
    for (size_t ii = 0; ii < _topTicks.size(); ii++) {
      out << _topTicks[ii];
      if (ii < _topTicks.size() - 1) {
        out << ", ";
      } else {
        out << endl;
      }
    }
  }

  if (_bottomTicks.size() > 0) {
    out << "  _bottomTicks       : ";
    for (size_t ii = 0; ii < _bottomTicks.size(); ii++) {
      out << _bottomTicks[ii];
      if (ii < _bottomTicks.size() - 1) {
        out << ", ";
      } else {
        out << endl;
      }
    }
  }

  if (_leftTicks.size() > 0) {
    out << "  _leftTicks         : ";
    for (size_t ii = 0; ii < _leftTicks.size(); ii++) {
      out << _leftTicks[ii];
      if (ii < _leftTicks.size() - 1) {
        out << ", ";
      } else {
        out << endl;
      }
    }
  }

  if (_rightTicks.size() > 0) {
    out << "  _rightTicks        : ";
    for (size_t ii = 0; ii < _rightTicks.size(); ii++) {
      out << _rightTicks[ii];
      if (ii < _rightTicks.size() - 1) {
        out << ", ";
      } else {
        out << endl;
      }
    }
  }

  out << "==========================================================" << endl;

}
  
