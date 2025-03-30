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

#include <rapmath/umath.h>
#include <toolsa/utim.h>
#include <qtplot/ColorMap.hh>
#include "WorldPlot.hh"

using namespace std;

////////////////////////////////////////
// default constructor

WorldPlot::WorldPlot() :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _scaledLabel(ScaledLabel::DistanceEng)
{

  _widthPixels = 1000;
  _heightPixels = 1000;

  _gridImage = nullptr;
  _overlayImage = nullptr;
  
  _xPixOffset = 0;
  _yPixOffset = 0;

  _xMinWorld = 0;
  _xMaxWorld = 1;
  _yMinWorld = 0;
  _yMaxWorld = 1;

  _plotWidth = 0;
  _plotHeight = 0;
  
  _leftMargin = 0;
  _rightMargin = 0;
  _topMargin = 0;
  _bottomMargin = 0;
  _titleTextMargin = 0;
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

  _backgroundColor = "black";
  _titleColor = "white";
  _axisLineColor = "white";
  _axisTextColor = "white";
  _gridColor = "gray";
  
  _computeTransform();
  
}

////////////////////////////////////////
// copy constructor

WorldPlot::WorldPlot(const WorldPlot &rhs):
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _scaledLabel(ScaledLabel::DistanceEng)

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
  
  _gridImage = nullptr;
  _overlayImage = nullptr;
  
  _xPixOffset = rhs._xPixOffset;
  _yPixOffset = rhs._yPixOffset;

  _proj = rhs._proj;
  
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

  _leftMargin = rhs._leftMargin;
  _rightMargin = rhs._rightMargin;
  _topMargin = rhs._topMargin;
  _bottomMargin = rhs._bottomMargin;
  _titleTextMargin = rhs._titleTextMargin;
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

void WorldPlot::setWorldLimitsX(double xMinWorld,
                                double xMaxWorld)
  
{
  
  if (_xMinWorld < _xMaxWorld) {
    _xMinWorld = MIN(xMinWorld, xMaxWorld);
    _xMaxWorld = MAX(xMinWorld, xMaxWorld);
  } else {
    _xMinWorld = MAX(xMinWorld, xMaxWorld);
    _xMaxWorld = MIN(xMinWorld, xMaxWorld);
  }
    
  _computeTransform();

}

void WorldPlot::setWorldLimitsY(double yMinWorld,
                                double yMaxWorld)
  
{
  
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
// set the y scale from the x scale - for plotting with aspect ratio 1.0

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

  // recompute transform
  
  _transform.reset();
  _transform.translate(_xMinPixel, _yMinPixel);
  _transform.scale(_xPixelsPerWorld, _yPixelsPerWorld);
  _transform.translate(-_xMinWorld, -_yMinWorld);
    
  _xMinWindow = getXWorld(_xPixOffset);
  _yMinWindow = getYWorld(_yPixOffset);
  _xMaxWindow = getXWorld(_xPixOffset + _widthPixels);
  _yMaxWindow = getYWorld(_yPixOffset + _heightPixels);

}

//////////////////////////////////////////////////////////////////////////
// set the x scale from the y scale - for plotting with aspect ratio 1.0

void WorldPlot::setXscaleFromYscale() 

{
  
  if (_yPixelsPerWorld * _xPixelsPerWorld < 0) {
    _xPixelsPerWorld = _yPixelsPerWorld * -1.0;
  } else {
    _xPixelsPerWorld = _yPixelsPerWorld;
  }

  double xMean = (_xMaxWorld + _xMinWorld) / 2.0;
  double xHalf = ((_xMaxPixel - _xMinPixel) / 2.0) / _xPixelsPerWorld;
  _xMinWorld = xMean - xHalf; 
  _xMaxWorld = xMean + xHalf;

  // recompute transform
  
  _transform.reset();
  _transform.translate(_xMinPixel, _yMinPixel);
  _transform.scale(_xPixelsPerWorld, _yPixelsPerWorld);
  _transform.translate(-_xMinWorld, -_yMinWorld);
    
  _xMinWindow = getXWorld(_xPixOffset);
  _yMinWindow = getYWorld(_yPixOffset);
  _xMaxWindow = getXWorld(_xPixOffset + _widthPixels);
  _yMaxWindow = getYWorld(_yPixOffset + _heightPixels);

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
// draw a point in pixel coords

void WorldPlot::drawPixelPoint(QPainter &painter,
                               double xx, double yy) 

{
	
  painter.save();
  QPointF qpt(xx, yy);
  painter.drawPoint(qpt);
  painter.restore();

}

///////////////////////////////
// draw a point in world coords

void WorldPlot::drawPoint(QPainter &painter,
                          double xx, double yy) 

{
	
  painter.save();
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);

  double xxx = getXPixel(xx);
  double yyy = getYPixel(yy);

  QPointF qpt(xxx, yyy);
  painter.drawPoint(qpt);

  painter.restore();

}

///////////////////////////////
// draw points in world coords

void WorldPlot::drawPoints(QPainter &painter,
                           const QVector<QPointF> &points) 

{
  
  painter.save();
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);

  QVector<QPointF> pts;
  for (int ii = 0; ii < points.size(); ii++) {
    double xxx = getXPixel(points[ii].x());
    double yyy = getYPixel(points[ii].y());
    QPointF pt(xxx, yyy);
    pts.push_back(pt);
  }

  painter.drawPoints(pts.data(), pts.size());

  painter.restore();

}

///////////////////////////////
// draw points in pixel coords

void WorldPlot::drawPixelPoints(QPainter &painter,
                                const QVector<QPointF> &points) 

{
  
  painter.save();
  painter.drawPoints(points.data(), points.size());
  painter.restore();

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

void WorldPlot::drawLines(QPainter &painter, const QVector<QPointF> &points) 

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

//////////////////////////////////////////////////
// fill a rectangle in world coords

void WorldPlot::fillRectangle(QPainter &painter,
                              const QBrush &brush,
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

//////////////////////////////////////////////////
// fill a rectangle in pixel coords

void WorldPlot::fillRectanglePixelCoords(QPainter &painter,
                                         const QBrush &brush,
                                         double xx, double yy,
                                         double width, double height) 
  
{
    
  painter.fillRect(xx, yy, width, height, brush);

}

////////////////////////////////////////////////////////
// fill a polygon

void WorldPlot::fillPolygon(QPainter &painter,
                            const QBrush &brush,
                            const QVector<QPointF> &points)
  
{
  
  painter.save();
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);
  
  // create a vector of points
  
  QVector<QPointF> pixPts;
  for (int ii = 0; ii < (int) points.size(); ii++) {
    QPointF pt(getXPixel(points[ii].x()), getYPixel(points[ii].y()));
    pixPts.push_back(pt);
  }
  // close polygon
  pixPts.push_back(pixPts[0]);
  
  // create a polygon from the points
  QPolygonF poly(pixPts);

  // add the polygon to a painter path
  QPainterPath path;
  path.addPolygon(poly);

  // fill the path

  painter.fillPath(path, brush);
  // drawPath(painter, path);

  painter.restore();

}

////////////////////////////////////////////////////////
// fill a polygon in pixel coords

void WorldPlot::fillPolygonPixelCoords(QPainter &painter,
                                       const QBrush &brush,
                                       const QVector<QPointF> &points)
  
{
  
  painter.save();
  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);
  
  // create a polygon from the points
  QPolygonF poly(points);

  // add the polygon to a painter path
  QPainterPath path;
  path.addPolygon(poly);
  
  // fill the path

  painter.fillPath(path, brush);
  // drawPath(painter, path);

  painter.restore();

}

///////////////////
// fill a trapezium

void WorldPlot::fillTrap(QPainter &painter,
                         const QBrush &brush,
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

//////////////////////////////////////
// fill the entire canvas with a color

void WorldPlot::fillCanvas(QPainter &painter,
                           const char *colorName) 
  
{
  
  double xx = _xPixOffset;
  double yy = _yPixOffset;
  double width = _widthPixels;
  double height = _heightPixels;

  QColor color(colorName);
  QBrush brush(color);
  
  painter.fillRect(xx, yy, width, height, brush);

}

void WorldPlot::fillCanvas(QPainter &painter,
                           const QBrush &brush) 

{
  
  double xx = _xPixOffset;
  double yy = _yPixOffset;
  double width = _widthPixels;
  double height = _heightPixels;
  
  painter.fillRect(xx, yy, width, height, brush);

}

///////////////////////////////////////////////////
// fill the margins with color

void WorldPlot::fillMargins(QPainter &painter,
                            const char *colorName) 
  
{

  QBrush brush(colorName);

  // left margin
  
  fillRectanglePixelCoords(painter, brush,
                           0, 0,
                           _leftMargin, _heightPixels);

  // right margin
  
  fillRectanglePixelCoords(painter, brush,
                           _xMaxPixel, 0,
                           _rightMargin + _colorScaleWidth + 1,
                           _heightPixels);
  
  // top margin
  
  fillRectanglePixelCoords(painter, brush,
                           0, 0,
                           _widthPixels, _topMargin);
  
  // bottom margin
  
  fillRectanglePixelCoords(painter, brush,
                           0, _yMinPixel,
                           _widthPixels, _bottomMargin);

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

////////////////////////////////////////////////////
// draw a general path clipped to within the margins
// points in screen coords

void WorldPlot::drawPathClippedScreenCoords(QPainter &painter,
                                            QPainterPath &path)

{

  painter.setClipRect(_xMinPixel, _yMaxPixel, _plotWidth,  _plotHeight);
  painter.drawPath(path);
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

//////////////////////////////////////
// draw centered text in world coords

void WorldPlot::drawTextCentered(QPainter &painter, const string &text,
                                 double text_x, double text_y) 

{

  drawText(painter, text, text_x, text_y, Qt::AlignCenter);

}

// draw text in screen coords
    
void WorldPlot::drawTextScreenCoords(QPainter &painter, const string &text,
                                     int text_ix, int text_iy,
                                     int flags)

{

  QRect tRect(painter.fontMetrics().tightBoundingRect(text.c_str()));
  QRect bRect(painter.fontMetrics().boundingRect(text_ix,
                                                 text_iy,
                                                 tRect.width() + 2,
                                                 tRect.height() + 2,
                                                 flags, text.c_str()));
  painter.drawText(bRect, flags, text.c_str());

}

/////////////////////////////////////
// draw rotated text in screen coords

void WorldPlot::drawRotatedTextScreenCoords(QPainter &painter, const string &text,
                                            int text_ix, int text_iy,
                                            int flags, double rotationDeg) 

{
  
  painter.save();
  painter.translate(text_ix, text_iy);
  painter.rotate(rotationDeg);
  
  QRect tRect(painter.fontMetrics().tightBoundingRect(text.c_str()));
  QRect bRect(painter.fontMetrics().boundingRect(0, 0,
                                                 tRect.width() + 2,
                                                 tRect.height() + 2,
                                                 flags, text.c_str()));
    
  painter.drawText(bRect, flags, text.c_str());
  
  painter.restore();
    
}

////////////////////////////////////////
// draw centered text in screen coords

void WorldPlot::drawTextCenteredScreenCoords(QPainter &painter, const string &text,
                                             int text_ix, int text_iy) 

{

  drawTextScreenCoords(painter, text, text_ix, text_iy, Qt::AlignCenter);

}

/////////////////////////////////////////////////////////////	
// Title
    
void WorldPlot::drawTitleTopCenter(QPainter &painter,
                                   const string &title) 

{

  painter.save();

  // set font
  
  QFont font(painter.font());
  font.setPointSizeF(_titleFontSize);
  font.setBold(true);
  painter.setFont(font);

  // get bounding rectangle
  
  QRect tRect(painter.fontMetrics().boundingRect(title.c_str()));

  // set location
  
  qreal xx = (qreal) ((_xMinPixel + _xMaxPixel - tRect.width()) / 2.0);
  qreal yy = (qreal) ((_topMargin - tRect.height()) / 4.0);
  QRectF bRect(xx, yy, tRect.width(), tRect.height());
    
  // draw the text
    
  painter.drawText(bRect, Qt::AlignTop, title.c_str());
  
  painter.restore();
  
}

///////////////////////	
// Titles top center
    
void WorldPlot::drawTitlesTopCenter(QPainter &painter,
                                    const vector<string> &titles) 
  
{
  
  painter.save();

  // compute line spacing
  
  QRect tRect0(painter.fontMetrics().tightBoundingRect("TITLE"));
  int startY = tRect0.height();
  int lineSpacing = (int) (tRect0.height() * 2);

  // set font
  
  QFont font(painter.font());
  font.setPointSizeF(_titleFontSize);
  font.setBold(true);
  painter.setFont(font);
  
  // draw sequential titles
  
  for (size_t ii = 0; ii < titles.size(); ii++) {
    string title(titles[ii]);
    QRect lRect(painter.fontMetrics().tightBoundingRect(title.c_str()));
    qreal xx = (qreal) ((_widthPixels - lRect.width()) / 2.0);
    qreal yy = startY + ii * lineSpacing;
    QRectF bRect(xx, yy, lRect.width() + 4, lRect.height() + 4);
    painter.drawText(bRect, Qt::AlignCenter, title.c_str());
  }
  
  painter.restore();

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

  QRectF bRect(0, 0, tRect.width() + 4, tRect.height() + 4);
  
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

  painter.save();

  QRect tRect(painter.fontMetrics().tightBoundingRect("TITLE"));
              
  qreal xx = (qreal) (_xMinPixel + _yAxisTickLen + _legendTextMargin);
  qreal yy = _yMaxPixel + _xAxisTickLen + _legendTextMargin;
  
  QFont font(painter.font());
  font.setPointSizeF(_legendFontSize);
  painter.setFont(font);
  
  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect lRect(painter.fontMetrics().boundingRect(legend.c_str()));
    QRectF bRect(xx, yy, lRect.width() + 6, lRect.height() + 6);
    painter.drawText(bRect, Qt::AlignTop, legend.c_str());
    yy += (_legendTextMargin + lRect.height());
  }
  
  painter.restore();

}

///////////////////////	
// Legends at top right
    
void WorldPlot::drawLegendsTopRight(QPainter &painter,
                                    const vector<string> &legends)

{

  painter.save();

  QRect tRect(painter.fontMetrics().tightBoundingRect("TITLE"));

  qreal yy = _yMaxPixel + _yAxisTickLen + tRect.height();
  
  QFont font(painter.font());
  font.setPointSizeF(_legendFontSize);
  painter.setFont(font);
  
  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect lRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    qreal xx = (qreal) (_xMaxPixel - _yAxisTickLen -
                        _legendTextMargin - lRect.width());
    QRectF bRect(xx, yy, lRect.width() + 4, lRect.height() + 4);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy += (_legendTextMargin + lRect.height());
  }

  painter.restore();

}

//////////////////////////	
// Legends at bottom left
    
void WorldPlot::drawLegendsBottomLeft(QPainter &painter,
                                      const vector<string> &legends) 

{
	
  painter.save();

  qreal xx = (qreal) (_xMinPixel + _yAxisTickLen + _legendTextMargin);
  qreal yy = (qreal) (_yMinPixel - _xAxisTickLen);

  QFont font(painter.font());
  font.setPointSizeF(_legendFontSize);
  painter.setFont(font);
  
  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect lRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    yy -= lRect.height();
    QRectF bRect(xx, yy, lRect.width() + 4, lRect.height() + 4);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy -= _legendTextMargin;
  }

  painter.restore();

}

///////////////////////////	
// Legends at bottom right
    
void WorldPlot::drawLegendsBottomRight(QPainter &painter,
                                       const vector<string> &legends) 

{
	
  painter.save();

  qreal yy = (qreal) (_yMinPixel - _xAxisTickLen);

  QFont font(painter.font());
  font.setPointSizeF(_legendFontSize);
  painter.setFont(font);
  
  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect lRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    qreal xx = (qreal) (_xMaxPixel - _yAxisTickLen -
                        _legendTextMargin - lRect.width());
    yy -= lRect.height();
    QRectF bRect(xx, yy, lRect.width() + 4, lRect.height() + 4);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy -= _legendTextMargin;
  }

  painter.restore();

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

  painter.save();
  
  QColor textColor(_axisTextColor.c_str());
  QBrush textBrush(textColor);
  QColor lineColor(_axisLineColor.c_str());
  
  // axis line
  
  if (doLine) {
    painter.setPen(lineColor);
    QLineF qlineGrid(_xMinPixel, _yMinPixel, _xMinPixel, _yMaxPixel);
    painter.drawLine(qlineGrid);
  }

  // font
  
  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  font.setBold(true);
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
               unitsRect.width() + 2, unitsRect.height() + 5);
  if (doLabels) {
    painter.setPen(textColor);
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
      painter.setPen(lineColor);
      painter.drawLine(qline);
    }
	    
    // grid
    
    if (doGrid) {
      if (ypix != _yMinPixel && ypix != _yMaxPixel) {
        painter.setPen(_gridColor.c_str());
        QLineF qlineGrid(_xMinPixel, ypix,
                         _xMaxPixel, ypix);
        painter.drawLine(qlineGrid);
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
        painter.setPen(textColor);
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }

  } // i
	
  painter.restore();

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
	
  painter.save();
  
  QColor textColor(_axisTextColor.c_str());
  QBrush textBrush(textColor);
  QColor lineColor(_axisLineColor.c_str());
  
  // axis line

  if (doLine) {
    painter.setPen(lineColor);
    QLineF qlineGrid(_xMaxPixel, _yMinPixel, _xMaxPixel, _yMaxPixel);
    painter.drawLine(qlineGrid);
  }

  // font

  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  font.setBold(true);
  painter.setFont(font);

  // axis units label
	
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel + _axisTextMargin);
  qreal unitsY = (qreal) (_yMaxPixel + (unitsRect.height() / 2));
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 5);
  if (doLabels) {
    painter.setPen(textColor);
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
      painter.setPen(lineColor);
      painter.drawLine(qline);
    }
	    
    // grid
    
    if (doGrid) {
      if (ypix != _yMinPixel && ypix != _yMaxPixel) {
        painter.setPen(_gridColor.c_str());
        QLineF qlineGrid(_xMinPixel, ypix,
                         _xMaxPixel, ypix);
        painter.drawLine(qlineGrid);
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
                  labelRect.width() + 2, labelRect.height() + 5);

    if ((fabs(labelY - unitsY) > labelRect.height() + _axisTextMargin) &&
        (fabs(labelY - _yMinPixel) > labelRect.height() + _axisTextMargin)) {
      if (doLabels) {
        painter.setPen(textColor);
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }
      
  }
	
  painter.restore();

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

  painter.save();
  
  QColor textColor(_axisTextColor.c_str());
  QBrush textBrush(textColor);
  QColor lineColor(_axisLineColor.c_str());
  
  // axis line
  
  if (doLine) {
    painter.setPen(lineColor);
    QLineF qlineGrid(_xMinPixel, _yMinPixel, _xMaxPixel, _yMinPixel);
    painter.drawLine(qlineGrid);
  }
    
  // font

  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  font.setBold(true);
  painter.setFont(font);

  // axis units label
	
  QRect htRect(painter.fontMetrics().tightBoundingRect("XXXyyygg"));
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel - unitsRect.width());
  qreal unitsY = (qreal) (_yMinPixel + htRect.height() - 2);
  if (_xAxisLabelsInside) {
    unitsY = (qreal) (_yMinPixel - htRect.height() - 2);
  }
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, htRect.height() + 5);
  if (doLabels) {
    painter.setPen(textColor);
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
      painter.setPen(lineColor);
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
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    if (((xpix + labelRect.width() / 2 + _axisTextMargin) < unitsX) &&
        ((xpix - labelRect.width() / 2 - _axisTextMargin) > _xMinPixel)) {
      qreal labelX = (qreal) (xpix - labelRect.width() / 2.0);
      qreal labelY = unitsY;
      QRectF bRect2(labelX, labelY,
                    labelRect.width() + 2, labelRect.height() + 5);
      if (doLabels) {
        painter.setPen(textColor);
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }
      
  }
	
  painter.restore();

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
	
  painter.save();
  
  QColor textColor(_axisTextColor.c_str());
  QBrush textBrush(textColor);
  QColor lineColor(_axisLineColor.c_str());
  
  // axis line
	
  if (doLine) {
    painter.setPen(lineColor);
    QLineF qlineGrid(_xMinPixel, _yMaxPixel, _xMaxPixel, _yMaxPixel);
    painter.drawLine(qlineGrid);
  }
	
  // font

  QFont font(painter.font());
  font.setPointSizeF(_axisLabelFontSize);
  font.setBold(true);
  painter.setFont(font);

  // axis units label
	
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel - unitsRect.width() / 2);
  qreal unitsY = (qreal) (_yMaxPixel - (unitsRect.height() + _axisTextMargin) - 2);
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 5);
  if (doLabels) {
    painter.setPen(textColor);
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
      painter.setPen(lineColor);
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
                    labelRect.width() + 2, labelRect.height() + 5);
      if (doLabels) {
        painter.setPen(textColor);
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }
      
  }
	
  painter.restore();

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
                   unitsRect.width() + 2, unitsRect.height() + 5);
  painter.drawText(bRectLeft, Qt::AlignCenter, units.c_str());

  qreal unitsXRight = (qreal) (_xMaxPixel + _axisTextMargin);
  QRectF bRectRight(unitsXRight, unitsY,
                    unitsRect.width() + 2, unitsRect.height() + 5);
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
                      labelRect.width() + 2, labelRect.height() + 5);
    QRectF bRect2Right(labelXRight, labelY,
                       labelRect.width() + 2, labelRect.height() + 5);
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
                             const DateTime &startTime,
                             const DateTime &endTime,
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
  
  vector<DateTime> ticks;
  for (int ii = 0; ii < nTicks; ii++) {
    DateTime tickTime(startUsecs + ii * intervalSecs);
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
               unitsRect.width() + 2, unitsRect.height() + 5);
  painter.drawText(bRect, Qt::AlignCenter, units.c_str());

  // tick marks

  for (size_t i = 0; i < ticks.size(); i++) {
    
    painter.setPen(lineColor);

    const DateTime &tickTime = ticks[i];
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
    snprintf(timeLabel, 1024, "%.2d:%.2d:%.2d",
             tickTime.getHour(), tickTime.getMin(), tickTime.getSec());
    QRect labelRect(painter.fontMetrics().tightBoundingRect(timeLabel));
    if (((pix + labelRect.width() / 2 + _axisTextMargin) < unitsX) &&
        ((pix - labelRect.width() / 2 - _axisTextMargin) > _xMinPixel)) {
      qreal labelX = (qreal) (pix - labelRect.width() / 2.0);
      qreal labelY = unitsY;
      QRectF bRect2(labelX, labelY,
                    labelRect.width() + 2, labelRect.height() + 5);
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
                    startRect.height() + 5);
  painter.drawText(startRect2, Qt::AlignCenter, startTimeLabel.c_str());
  
  QRect 
    endRect(painter.fontMetrics().tightBoundingRect(endTimeLabel.c_str()));
  qreal endX = _xMaxPixel - endRect.width();
  qreal endY = startY;
  QRectF endRect2(endX, endY, endRect.width() + 2, endRect.height() + 5);
  painter.drawText(endRect2, Qt::AlignCenter, endTimeLabel.c_str());

  // restore painter to original state

  painter.restore();
      
} // drawTimeAxes

////////////////////////////////
// distance ticks on time axis

void WorldPlot::drawDistanceTicks(QPainter &painter,
                                  const DateTime &startTime,
                                  const vector<double> &tickDists,
                                  const vector<DateTime> &tickTimes,
                                  const QColor &lineColor,
                                  const QColor &textColor,
                                  const QFont &valuesFont)
  
{
	
  // save the painter state
  
  painter.save();
  
  // loop through ticks, ignoring the first one

  double prevPix = -9999;

  for (size_t i = 0; i < tickDists.size(); i++) {
    
    const DateTime &tickTime = tickTimes[i];
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
    snprintf(distLabel, 1024, "%gkm", distVal);
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
                  labelRect.width() + 2, labelRect.height() + 5);
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

////////////////////////////////////////////////////
// get axis label given value and delta
    
string WorldPlot::getAxisLabel(double delta, double val)

{

  char text[1024];
  delta = fabs(delta);
  
  if (delta >= 1.0) {
    snprintf(text, 1024, "%.0f", val);
  } else if (delta >= 0.1) {
    snprintf(text, 1024, "%.1f", val);
  } else if (delta >= 0.01) {
    snprintf(text, 1024, "%.2f", val);
  } else if (delta >= 0.001) {
    snprintf(text, 1024, "%.3f", val);
  } else if (delta >= 0.0001) {
    snprintf(text, 1024, "%.4f", val);
  } else {
    snprintf(text, 1024, "%.5f", val);
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

//////////////////////////////////////////////////////////
// update X and Y pixel scales to minimize distortion

void WorldPlot::updatePixelScales()
{

  // compute pixel space transform

  _plotWidth = _widthPixels - _leftMargin - _rightMargin - _colorScaleWidth;
  _plotHeight = _heightPixels - _topMargin - _bottomMargin;
  
  _xMinPixel = _leftMargin + _xPixOffset;
  _xMaxPixel = _xMinPixel + _plotWidth - 1;
  _yMaxPixel = _topMargin + _yPixOffset;
  _yMinPixel = _yMaxPixel + _plotHeight - 1;

  // compute world coords per pixel in each dirn

  _xPixelsPerWorld =
    (_xMaxPixel - _xMinPixel) / (_xMaxWorld - _xMinWorld);
  _yPixelsPerWorld =
    (_yMaxPixel - _yMinPixel) / (_yMaxWorld - _yMinWorld);

  // correct y for aspect change in LATLON projection
  // to preserve the plotted aspect ratio
  
  double aspectCorr = 1.0;
  if (_proj.getProjType() == Mdvx::PROJ_LATLON) {
    double meanLat = (_yMaxWorld + _yMinWorld) / 2.0;
    aspectCorr = cos(meanLat * DEG_TO_RAD);
  }
  _yPixelsPerWorld /= aspectCorr;

  // adjust the world coords of the corners, based on the shape

  double plotAspect = fabs(_xPixelsPerWorld / _yPixelsPerWorld);
  
  if (plotAspect < aspectCorr) {
    // adjust y pixel scale
    _xPixelsPerWorld = _yPixelsPerWorld * aspectCorr;
    if (_yPixelsPerWorld > 0) {
      _yPixelsPerWorld = fabs(_xPixelsPerWorld);
    } else {
      _yPixelsPerWorld = fabs(_xPixelsPerWorld) * -1.0;
    }
    double yMean = (_yMaxWorld + _yMinWorld) / 2.0;
    double yHalf = ((_yMaxPixel - _yMinPixel) / 2.0) / _yPixelsPerWorld;
    _yMinWorld = yMean - yHalf; 
    _yMaxWorld = yMean + yHalf;
  } else {
    // adjust x pixel scale
    _yPixelsPerWorld = _xPixelsPerWorld / aspectCorr;
    if (_xPixelsPerWorld > 0) {
      _xPixelsPerWorld = fabs(_yPixelsPerWorld);
    } else {
      _xPixelsPerWorld = fabs(_yPixelsPerWorld) * -1.0;
    }
    double xMean = (_xMaxWorld + _xMinWorld) / 2.0;
    double xHalf = ((_xMaxPixel - _xMinPixel) / 2.0) / _xPixelsPerWorld;
    _xMinWorld = xMean - xHalf; 
    _xMaxWorld = xMean + xHalf;
  }

  // recompute
  
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

  _computeTransform();
  
}

/////////////////////////////////////////////////////
// draw the color scale

void WorldPlot::drawColorScale(const ColorMap &colorMap,
                               QPainter &painter,
                               int unitsFontSize)
  
{

  const std::vector<ColorMap::CmapEntry> &cmap = colorMap.getEntries();

  int pltHt = _plotHeight;
  int width = _colorScaleWidth - _rightMargin;
  int xStart = _xPixOffset + _widthPixels - _colorScaleWidth;
  int yStart = _topMargin + _yMaxPixel + unitsFontSize;
  size_t nHts = cmap.size() + 1; // leave some space at top and bottom
  double patchHt = (double)(pltHt) / nHts;
  int iPatchHt = (int) patchHt;

  // compute y locations of the color patches
  
  vector<double> topY;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    double yy = yStart + pltHt - (int) ((ii + 2) * patchHt + (patchHt / 2));
    topY.push_back(yy);
  } // ii
  
  // fill the swatches with the color
  
  painter.save();
  painter.setPen(Qt::SolidLine);
  int scaleYTop = 0, scaleYBot = 0;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    QColor color(entry.red, entry.green, entry.blue);
    painter.setBrush(color);
    QRectF r(xStart, topY[ii], width, patchHt+1);
    painter.fillRect(r, color);
    if (ii == 0) {
      scaleYBot = topY[ii] + patchHt;
    } else if (ii == cmap.size() - 1) {
      scaleYTop = topY[ii];
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
  QFont scaledFont(defaultFont);
  scaledFont.setPointSizeF(unitsFontSize);
  painter.setFont(scaledFont);
  
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
      double yy = _yMaxPixel + scaleYBot - scaleHeight * label.position;
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
      int iyy = (int) (topY[ii] + patchHt * 0.5);
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
    int iyy = (int) (topY[cmap.size()-1] - patchHt * 0.5);
    painter.drawText(xStart, iyy, width, iPatchHt, 
                     Qt::AlignVCenter | Qt::AlignHCenter, 
                     label);
  }

  // add Units label
  
  string units(colorMap.getUnits());
  if (units.size() > 0) {
    
    QFont ufont(painter.font());
    ufont.setPointSizeF(unitsFontSize);
    painter.setFont(ufont);
    
    QRect tRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
    int iyy = (int) (topY[cmap.size()-1] - patchHt * 1.0);
    int ixx = _xPixOffset + _widthPixels - width - _rightMargin / 2;
    QString qunits(units.c_str());
    painter.drawText(ixx, iyy, width, tRect.height() + 6, 
                     Qt::AlignTop | Qt::AlignHCenter, qunits);

  }

  // restore state

  painter.restore();

}

/////////////////////////////////////////////////////
// create images with size of the plot

void WorldPlot::_createImage(QImage* &image)
{
  if (image == nullptr) {
    image = new QImage(_widthPixels, _heightPixels, QImage::Format_ARGB32);
  } else {
    if (image->width() != _widthPixels ||
        image->height() != _heightPixels) {
      delete image;
      image = new QImage(_widthPixels, _heightPixels, QImage::Format_ARGB32);
    }
  }
}
                               
// get images after rendering
  
const QImage *WorldPlot::getGridImage()
{
  if (!_gridImage) {
    _createImage(_gridImage);
  }
  return _gridImage;
}

const QImage *WorldPlot::getOverlayImage()
{
  if (!_overlayImage) {
    _createImage(_overlayImage);
  }
  return _overlayImage;
}

/////////////////////////////////////////////////////
// render a data grid in Cartesian rectangular pixels
// used if there is no distortion

void WorldPlot::renderGridRect(int page,
                               MdvReader *mr,
                               time_t start_time,
                               time_t end_time,
                               bool is_overlay_field)
  
{

  cerr << "RRRRRRRRRRRRRRR ====>> renderGridRect, page: " << page << endl;

  // check that image canvas is the correct size
  
  _createImage(_gridImage);
  QPainter painter(_gridImage);

  // fill with background color
  
  fillCanvas(painter, _params.background_color);
   
  // the data projection type and plot projection type are the same
  // so we can use the (x,y) locations unchanged

  // compute the location of the vertices
  // these are the cell limits in (x, y)

  vector< vector<QPointF> > vertices;

  int nx = mr->h_fhdr.nx;
  int ny = mr->h_fhdr.ny;
  double dy = mr->h_fhdr.grid_dy;
  double lowy = mr->h_fhdr.grid_miny - dy / 2.0;
  double dx = mr->h_fhdr.grid_dx;
  double lowx = mr->h_fhdr.grid_minx - dx / 2.0;

  double yy = lowy;
  for(int iy = 0; iy <= ny; iy++, yy += dy) {
    vector<QPointF> row;
    double xx = lowx;
    for(int ix = 0; ix <= nx; ix++, xx += dx) {
      QPointF pt = getPixelPointF(xx, yy);
      row.push_back(pt);
    } // ix
    vertices.push_back(row);
  } // iy
  
  // plot the rectangles for each grid cell

  fl32 *val = mr->h_fl32_data;
  fl32 miss = mr->h_fhdr.missing_data_value;
  fl32 bad = mr->h_fhdr.bad_data_value;
  
  for(int iy = 0; iy < mr->h_fhdr.ny; iy++) {
    for(int ix = 0; ix < mr->h_fhdr.nx; ix++, val++) {
      fl32 fval = *val;
      if (fval == miss || fval == bad) {
        continue;
      }
      if (!mr->colorMap->withinValidRange(fval)) {
        continue;
      }
      const QBrush *brush = mr->colorMap->dataBrush(fval);
      double xx = vertices[iy+1][ix].x();
      double yy = vertices[iy+1][ix].y();
      double width = vertices[iy][ix+1].x() - vertices[iy][ix].x() + 1;
      double height = vertices[iy][ix].y() - vertices[iy+1][ix].y() + 1;
      fillRectanglePixelCoords(painter, *brush, xx, yy, width, height);
    } // ix
  } // iy

  // fill margins with background color

  fillMargins(painter, _params.background_color);

#ifdef NOTYET
  
  int i,j;
  int ht,wd;               /* Dims of data rectangles  */
  int startx,endx;        /* pixel limits of data area */
  int starty,endy;        /* pixel limits of data area */
  long num_points;
  render_method_t render_method;
  double x_dproj, y_dproj;
  double val;
  int x_start[MAX_COLS + 4];    /* canvas rectangle begin  coords */
  int y_start[MAX_ROWS + 4];    /* canvas  rectangle begin coords */
  double r_ht, r_wd;        /*  data point rectangle dims */
  unsigned short *ptr;

  ptr = (unsigned short *) mr->h_data;
  if(ptr == NULL) {
    cerr << "ERROR - WorldPlot::renderGridRect()" << endl;
    cerr << "  NULL data" << endl;
    return;
  }

  // Use unused parameters
  start_time = 0; end_time = 0;
     
  // Establish with method we're going to use to render the grid.
  render_method = (render_method_t) mr->render_method;
  num_points = mr->h_fhdr.nx * mr->h_fhdr.ny;
  if(render_method == DYNAMIC_CONTOURS) {
    if(num_points < _params.dynamic_contour_threshold) {
      render_method = FILLED_CONTOURS;
    } else {
      render_method = POLYGONS;
    }
  }
  
  // int psc = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS);
  int psc = 0;
  
  // Images don't get countoured
  if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32)render_method = POLYGONS;

  /* Calculate Data to Screen Mapping */
  grid_to_disp_proj(mr,0,0,&x_dproj,&y_dproj);
  if(render_method == POLYGONS) x_dproj -= mr->h_fhdr.grid_dx/2.0;    /* expand grid coord by half width */
  if(render_method == POLYGONS) y_dproj -= mr->h_fhdr.grid_dy/2.0;
  disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&startx,&starty);

  grid_to_disp_proj(mr,mr->h_fhdr.nx-1,mr->h_fhdr.ny-1,&x_dproj,&y_dproj);
  if(render_method == POLYGONS) x_dproj += mr->h_fhdr.grid_dx/2.0;    /* expand grid coord by half width */
  if(render_method == POLYGONS) y_dproj += mr->h_fhdr.grid_dy/2.0;
  disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&endx,&endy);

  /* get data point rectangle size */
  if(render_method == POLYGONS) {
    r_ht =  (double)(ABSDIFF(starty,endy))  / ((double) mr->h_fhdr.ny);
    r_wd =  (double)(endx - startx)/ ((double) mr->h_fhdr.nx);    
  } else {
    r_ht =  (double)(ABSDIFF(starty,endy))  / ((double) mr->h_fhdr.ny - 1.0);
    r_wd =  (double)(endx - startx)/ ((double) mr->h_fhdr.nx - 1.0);    
  }

  //fprintf(stderr,"X: %d to %d  Y: %d to %d   WD,HT: %g %g   NX,NY: %d,%d\n",
  //		  startx,endx,starty,endy,r_wd,r_ht,mr->h_fhdr.nx,mr->h_fhdr.ny);

  /* Calc starting coords for the X,Y array */
  for(j=0;j <= mr->h_fhdr.nx; j++) {
    x_start[j] = (int) (((double) j * r_wd) + startx);
    if(x_start[j] < 0) x_start[j] = 0;
    if(x_start[j] > gd.h_win.can_dim.width) x_start[j] = gd.h_win.can_dim.width;
  }

  for(i= 0; i <= mr->h_fhdr.ny; i++) {
    y_start[i] = (int) (starty - ((double) i * r_ht));
    if(y_start[i] < 0) y_start[i] = 0;
    if(y_start[i] >= gd.h_win.can_dim.height) y_start[i] = gd.h_win.can_dim.height -1;
  }


  if(mr->num_display_pts <=0) mr->num_display_pts = mr->h_fhdr.ny * mr->h_fhdr.nx;

  ht = (int) (r_ht + 1.0); 
  wd = (int) (r_wd + 1.0);

  if (_params.clip_overlay_fields) {
      
    if(gd.debug2) printf("Drawing Rectangle Fill Overlay image field: %s\n",
                         mr->field_label);

    // X windows paints downward on the screen. Move starting point to the top of row.
    for(i= 0; i < mr->h_fhdr.ny; i++) {
      y_start[i] -= (ht -1);
    }
      
    // An Image
    if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {

#ifdef NOTYET
      
      ui32 pixel;
      ui32 *uptr;
      uptr = (ui32 *) mr->h_data;
        
      XStandardColormap best_map;
      if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
        // fprintf(stderr,"  (Try running 'xstdcmap -best')\n");
        // assume 24-bit depth, 8-bit colors
        fprintf(stderr,"WARNING - XGetStandardColormap() failed\n");
        fprintf(stderr,"  Setting base_pixel = 0\n"); 
        fprintf(stderr,"  Setting   red_mult = 65536, green_mult = 256, blue_mult = 1\n"); 
        fprintf(stderr,"  Setting   red_max  = 255,   green_max  = 255, blue_max  = 255\n"); 
        best_map.base_pixel = 0;
        best_map.red_mult = 65536;
        best_map.green_mult = 256;
        best_map.blue_mult = 1;
        best_map.red_max = 255;
        best_map.green_max = 255;
        best_map.blue_max = 255;
        // return CIDD_FAILURE;
      }
      uptr =  (ui32 * ) mr->h_data;

      for(i= 0; i < mr->h_fhdr.ny; i++) {
        for(j=0;j< mr->h_fhdr.nx; j++) {
          if(MdvGetA(*uptr) != 0) {
            pixel = best_map.base_pixel + 
              ((ui32) (0.5 + ((_params.image_inten * MdvGetR(*uptr) / 255.0) * best_map.red_max)) * best_map.red_mult) +
              ((ui32) (0.5 + ((_params.image_inten * MdvGetG(*uptr) / 255.0) * best_map.green_max)) * best_map.green_mult) +
              ((ui32) (0.5 + ((_params.image_inten * MdvGetB(*uptr) / 255.0) * best_map.blue_max)) * best_map.blue_mult);

            XSetForeground(gd.dpy,gd.def_gc, pixel);
            XFillRectangle(gd.dpy,xid,gd.def_gc, x_start[j],y_start[i],wd,ht);
          }
          uptr++;
        }
      }

#endif

    } else { // if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32)
      
      mr->num_display_pts = 0;
      for(i= 0; i < mr->h_fhdr.ny; i++) {
        for(j=0;j< mr->h_fhdr.nx; j++) {
          if(mr->h_vcm.val_gc[*ptr] != NULL) {
            val =  (mr->h_fhdr.scale * *ptr) + mr->h_fhdr.bias;
            if(val >= mr->overlay_min && val <= mr->overlay_max) 
              XFillRectangle(gd.dpy,xid,mr->h_vcm.val_gc[*ptr],x_start[j],y_start[i],wd,ht);
            mr->num_display_pts++;
          }
          ptr++;
        }
      }
    }

  } else { // if (_params.clip_overlay_fields) ..
	 
    // Handle case where contours really don't apply (vertical sides)
    if(mr->h_fhdr.min_value == mr->h_fhdr.max_value ) render_method = POLYGONS;
    switch (render_method) {

      case POLYGONS:
        if(( PseudoColor == psc ) && !_params.draw_main_on_top && mr->num_display_pts > _params.image_fill_threshold) {
          draw_filled_gridImage(xid,x_start,y_start,mr);
        } else {
          if(gd.debug2) printf("Drawing Rectangle Fill image, field: %s \n",
                               mr->field_label);
          mr->num_display_pts = 0;
             
          // X windows paints downward on the screen. Move starting point to the top of row.
          for(i= 0; i < mr->h_fhdr.ny; i++) y_start[i]-= (ht-1);
             
          // An Image
          if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
            ui32 pixel;
            ui32 *uptr;
            uptr = (ui32 *) mr->h_data;
               
            XStandardColormap best_map;
            if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
              // try to fix the problem
              safe_system("xstdcmap -best",_params.simple_command_timeout_secs);
              if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
                fprintf(stderr,"Failed XGetStandardColormap!\n");
                fprintf(stderr,"Can't Render RGB images - Try Running X Server in 24 bit mode\n");
                fprintf(stderr,"Run 'xstdcmap -best -verbose' to see the problem\n");
                // assume 24-bit depth, 8-bit colors
                fprintf(stderr,"Setting base_pixel = 0, red_mult = 65536, green_mult = 256, blue_mult = 1\n");
                best_map.base_pixel = 0;
                best_map.red_mult = 65536;
                best_map.green_mult = 256;
                best_map.blue_mult = 1;
                // return CIDD_FAILURE;
              }
            } // if(! XGetStandardColormap ...

            for(i= 0; i < mr->h_fhdr.ny; i++) {
              for(j=0;j< mr->h_fhdr.nx; j++) {
                if(MdvGetA(*uptr) != 0) {
                  pixel = best_map.base_pixel + 
                    ((ui32) (0.5 + ((_params.image_inten * MdvGetR(*uptr) / 255.0) * best_map.red_max)) * best_map.red_mult) +
                    ((ui32) (0.5 + ((_params.image_inten * MdvGetG(*uptr) / 255.0) * best_map.green_max)) * best_map.green_mult) +
                    ((ui32) (0.5 + ((_params.image_inten * MdvGetB(*uptr) / 255.0) * best_map.blue_max)) * best_map.blue_mult);
                     
                  XSetForeground(gd.dpy,gd.def_gc, pixel);
                  XFillRectangle(gd.dpy,xid,gd.def_gc, x_start[j],y_start[i],wd,ht);
                  mr->num_display_pts++;
                }
                uptr++;
              }
            } // i
               
          } else { // if(mr->h_fhdr.encoding_type ...
               
            //
            // Not an image
            //
            for(i= 0; i < mr->h_fhdr.ny; i++) {
              for(j=0;j< mr->h_fhdr.nx; j++) {
                if(mr->h_vcm.val_gc[*ptr] != NULL) {
                  if ( is_overlay_field ) {
                    //
                    // If it is an overlay field, need to check value against
                    // min, max specified in GUI. If it is not a value we want to plot,
                    // just increment the pointer and move on. Niles Oien 5/17/2010
                    //
                    val =  (mr->h_fhdr.scale * *ptr) + mr->h_fhdr.bias;
                    if(val < mr->overlay_min || val > mr->overlay_max){
                      ptr++; continue;
                    }
                  }
                  XFillRectangle(gd.dpy,xid,mr->h_vcm.val_gc[*ptr],x_start[j],y_start[i],wd,ht);
                  mr->num_display_pts++;
                }
                ptr++;
              }
            }
          } // if(mr->h_fhdr.encoding_type ...
             
          if(gd.debug2) {
            printf("NUM POLYGONS: %d of %ld \n",
                   mr->num_display_pts,
                   mr->h_fhdr.nx*mr->h_fhdr.ny);
          }
             
        }
        break;
           
      case FILLED_CONTOURS:
        if(gd.debug2) printf("Drawing Filled Contour image: field %s\n",
                             mr->field_label);
        if (gd.layers.use_alt_contours) {
          RenderFilledPolygons(xid, mr);
        } else {
          draw_filled_contours(xid,x_start,y_start,mr);
        }
        break;

      case DYNAMIC_CONTOURS:
      case LINE_CONTOURS:
        break;
        
    } // switch (render_method)

  } // if (_params.clip_overlay_fields) ..

#endif
  
}

//////////////////////////////////////////////////////////
// render a data grid which is distorted
// i.e. where the data and display projection do not match

void WorldPlot::renderGridDistorted(int page,
                                    MdvReader *mr,
                                    time_t start_time,
                                    time_t end_time,
                                    bool is_overlay_field)
  
{

  cerr << "DDDDDDDDDDDDDDDDDDD ====>> renderGridDistorted, page: " << page << endl;

  // check that image canvas is the correct size
  
  _createImage(_gridImage);
  QPainter painter(_gridImage);
  
  // fill with background color
  
  fillCanvas(painter, _params.background_color);
   
  // the data projection type and plot projection type are not the same
  // so we need to compute the lat/lon of each corner of the grid cells

  // compute the location of the vertices
  // these are the cell limits in (x, y)
  
  vector< vector<QPointF> > vertices;
  
  int nx = mr->h_fhdr.nx;
  int ny = mr->h_fhdr.ny;
  double dy = mr->h_fhdr.grid_dy;
  double lowy = mr->h_fhdr.grid_miny - dy / 2.0;
  double dx = mr->h_fhdr.grid_dx;
  double lowx = mr->h_fhdr.grid_minx - dx / 2.0;

  // compute the vertices

  // cerr << "=============>> Data proj" << endl;
  // mr->proj->print(cerr);
  // cerr << "==================================================" << endl;
  // cerr << "=============>> Display proj" << endl;
  // _proj.print(cerr);
  // cerr << "==================================================" << endl;
  
  double yy = lowy;
  for(int iy = 0; iy <= ny; iy++, yy += dy) {
    vector<QPointF> row;
    double xx = lowx;
    for(int ix = 0; ix <= nx; ix++, xx += dx) {
      // compute lat/lon from data projection
      double lat, lon;
      mr->proj->xy2latlon(xx, yy, lat, lon);
      // compute x(x,y) in display projection
      double xx1, yy1;
      _proj.latlon2xy(lat, lon, xx1, yy1);
      // cerr << "xxxxxxxx xx, yy, lat, lon, xx1, yy1: "
      //      << xx << ", " << yy << ", "
      //      << lat << ", " << lon << ", "
      //      << xx1 << ", " << yy1 << endl;
      // set pixel space point
      QPointF pt = getPixelPointF(xx1, yy1);
      row.push_back(pt);
    } // ix
    vertices.push_back(row);
  } // iy
  
  // plot the polygons for each grid cell

  fl32 *val = mr->h_fl32_data;
  fl32 miss = mr->h_fhdr.missing_data_value;
  fl32 bad = mr->h_fhdr.bad_data_value;
  
  for(int iy = 0; iy < mr->h_fhdr.ny; iy++) {
    for(int ix = 0; ix < mr->h_fhdr.nx; ix++, val++) {
      fl32 fval = *val;
      if (fval == miss || fval == bad) {
        continue;
      }
      if (!mr->colorMap->withinValidRange(fval)) {
        continue;
      }
      const QBrush *brush = mr->colorMap->dataBrush(fval);
      QVector<QPointF> points;
      points.push_back(vertices[iy][ix]);
      points.push_back(vertices[iy][ix+1]);
      points.push_back(vertices[iy+1][ix+1]);
      points.push_back(vertices[iy+1][ix]);
      fillPolygonPixelCoords(painter, *brush, points);
    } // ix
  } // iy
  
  // fill margins with background color

  fillMargins(painter, _params.background_color);

}

/////////////////////////////////////////////////////
// render polar radar grid

void WorldPlot::renderGridRadarPolar(int page,
                                     MdvReader *mr,
                                     time_t start_time,
                                     time_t end_time,
                                     bool is_overlay_field)
  
{

  cerr << "====>> radar polar radar polar, page: " << page << endl;
  
  // check that image canvas is the correct size
  
  _createImage(_gridImage);
  QPainter painter(_gridImage);
  
  // fill with background color
  
  fillCanvas(painter, _params.background_color);
   
  // the data projection type and plot projection type are not the same
  // so we need to compute the lat/lon of each corner of the grid cells

  // compute the location of the vertices
  // these are the cell limits in (x, y)
  
  vector< vector<QPointF> > vertices;
  
  int nx = mr->h_fhdr.nx;
  int ny = mr->h_fhdr.ny;
  double dy = mr->h_fhdr.grid_dy;
  double lowy = mr->h_fhdr.grid_miny - dy / 2.0;
  double dx = mr->h_fhdr.grid_dx;
  double lowx = mr->h_fhdr.grid_minx - dx / 2.0;
  double elevDeg = mr->h_fhdr.grid_minz;
  if (!_params.use_cosine_correction) {
    // set elevation angle to 0 so cosine correction is not applied
    elevDeg = 0.0;
  }

  // compute the vertices

  // cerr << "=============>> Data proj" << endl;
  // mr->proj->print(cerr);
  // cerr << "==================================================" << endl;
  // cerr << "=============>> Display proj" << endl;
  // _proj.print(cerr);
  // cerr << "==================================================" << endl;
  
  double yy = lowy;
  for(int iy = 0; iy <= ny; iy++, yy += dy) {
    vector<QPointF> row;
    double xx = lowx;
    for(int ix = 0; ix <= nx; ix++, xx += dx) {
      // compute lat/lon from data projection
      double lat, lon;
      mr->proj->xy2latlon(xx, yy, lat, lon, elevDeg);
      // compute x(x,y) in display projection
      double xx1, yy1;
      _proj.latlon2xy(lat, lon, xx1, yy1, elevDeg);
      // set pixel space point
      QPointF pt = getPixelPointF(xx1, yy1);
      row.push_back(pt);
    } // ix
    vertices.push_back(row);
  } // iy
  
  // plot the polygons for each grid cell

  fl32 *val = mr->h_fl32_data;
  fl32 miss = mr->h_fhdr.missing_data_value;
  fl32 bad = mr->h_fhdr.bad_data_value;
  
  for(int iy = 0; iy < mr->h_fhdr.ny; iy++) {
    for(int ix = 0; ix < mr->h_fhdr.nx; ix++, val++) {
      fl32 fval = *val;
      if (fval == miss || fval == bad) {
        continue;
      }
      if (!mr->colorMap->withinValidRange(fval)) {
        continue;
      }
      const QBrush *brush = mr->colorMap->dataBrush(fval);
      QVector<QPointF> points;
      points.push_back(vertices[iy][ix]);
      points.push_back(vertices[iy][ix+1]);
      points.push_back(vertices[iy+1][ix+1]);
      points.push_back(vertices[iy+1][ix]);
      fillPolygonPixelCoords(painter, *brush, points);
    } // ix
  } // iy
  
  // fill margins with background color

  fillMargins(painter, _params.background_color);

}

/////////////////////////////////////////////////////
// draw the overlays - maps, range rings etc.

void WorldPlot::drawOverlays(bool ringsEnabled,
                             bool azLinesEnabled,
                             double ringSpacing)
  
{

  // check that overlay canvas is the correct size
  
  _createImage(_overlayImage);
  
  // make transparent
  
  _overlayImage->fill(Qt::transparent);
  
  // render maps
  
  QPainter painter(_overlayImage);
  _drawMaps(painter);

  // render range rings and az line

  if (ringsEnabled || azLinesEnabled) {
    _drawRangeRingsAndAzLines(painter,
                              ringsEnabled, azLinesEnabled,
                              ringSpacing);
  }

}

/*************************************************************************
 * draw map overlays
 */

void WorldPlot::_drawMaps(QPainter &painter)

{

  painter.save();

  // Loop throughs maps
  
  for(int ii = _gd.num_map_overlays - 1; ii >= 0; ii--) {
    
    if(!_gd.overlays[ii]->active ||
       (_gd.overlays[ii]->detail_thresh_min > _gd.h_win.km_across_screen) ||
       (_gd.overlays[ii]->detail_thresh_max < _gd.h_win.km_across_screen))  {
      continue;
    }
      
    MapOverlay_t *ov = _gd.overlays[ii];

    // create the pen for this map
    
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(ov->line_width);
    pen.setColor(ov->color_name.c_str());
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    
    QFont mfont(painter.font());
    mfont.setPointSizeF(_params.maps_font_size);
    painter.setFont(mfont);
    
    // Draw labels
    
    for(int jj = 0; jj < ov->num_labels; jj++) {
      if(ov->geo_label[jj]->proj_x <= -32768.0) {
        continue;
      }
      drawText(painter,
               ov->geo_label[jj]->display_string,
               ov->geo_label[jj]->proj_x,
               ov->geo_label[jj]->proj_y,
               Qt::AlignCenter);
    } // jj
    
    // draw icons
    
    for(int jj = 0; jj < ov->num_icons; jj++) {
      
      Geo_feat_icon_t *ic = ov->geo_icon[jj];
      if(ic->proj_x <= -32768.0) {
        continue;
      }
      
      int ixx = getIxPixel(ic->proj_x);
      int iyy = getIyPixel(ic->proj_y);

      // draw the icon

      int minIy = 1.0e6;
      int maxIy = -1.0e6;
      for(int kk = 0; kk < ic->icon->num_points - 1; kk++) {
        if ((ic->icon->x[kk] == 32767) ||
            (ic->icon->x[kk+1] == 32767)) {
          continue;
        }
        double iconScale = 1.0;
        int ix1 = ixx + (int) (ic->icon->x[kk] * iconScale + 0.5);
        int ix2 = ixx + (int) (ic->icon->x[kk+1] * iconScale + 0.5);
        int iy1 = iyy + (int) (ic->icon->y[kk] * iconScale + 0.5);
        int iy2 = iyy + (int) (ic->icon->y[kk+1] * iconScale + 0.5);
        minIy = std::min(minIy, iy1);
        minIy = std::min(minIy, iy2);
        maxIy = std::max(maxIy, iy1);
        maxIy = std::max(maxIy, iy2);
        drawPixelLine(painter, ix1, iy1, ix2, iy2);
      } // kk
      
      // add icon label
      
      painter.save();
      if(_params.map_font_background == Params::MAP_FONT_BACKGROUND_TRANSPARENT) {
        painter.setBackgroundMode(Qt::TransparentMode);
      } else {
        painter.setBackgroundMode(Qt::OpaqueMode);
      }
      // int alignment = Qt::AlignHCenter | Qt::AlignBottom;
      // if (ic->text_y < 0) {
      //   alignment = Qt::AlignHCenter | Qt::AlignTop;
      // }
      // alignment = Qt::AlignCenter;
      if (ic->text_y < 0) {
        drawTextScreenCoords(painter, ic->label,
                             ixx + ic->text_x,
                             maxIy - ic->text_y,
                             Qt::AlignCenter);
      } else {
        drawTextScreenCoords(painter, ic->label,
                             ixx + ic->text_x,
                             minIy - ic->text_y,
                             Qt::AlignCenter);
      }
      // cerr << "IIIIIIII text_x, text_y, label: "
      //      << ic->text_x << ", " << ic->text_y
      //      << ", " << ic->label << endl;
      painter.restore();
      
    } // jj

    // draw polylines
    
    for(int jj = 0; jj < ov->num_polylines; jj++) {
      
      Geo_feat_polyline_t *poly = ov->geo_polyline[jj];
      QPainterPath polyPath;
      bool doMove = true;
      
      for(int ll = 0; ll < poly->num_points; ll++) {
        
        double proj_x = poly->proj_x[ll];
        double proj_y = poly->proj_y[ll];
        
        bool validPoint = true;
        if (fabs(proj_x) > 32767 || fabs(proj_y) > 32767) {
          validPoint = false;
        }

        if (!validPoint || (ll == 0)) {
          doMove = true;
        }

        if (validPoint) {
          QPointF point = getPixelPointF(proj_x, proj_y);
          if (doMove) {
            polyPath.moveTo(point);
            doMove = false;
          } else {
            polyPath.lineTo(point);
          }
        }

      } // ll

      drawPathClippedScreenCoords(painter, polyPath);
      // drawPath(painter, polyPath);

    } // jj
      
  } // ii
  
  painter.restore();

}

/*************************************************************************
 * _drawRingsAndAzLines()
 *
 * draw rings for polar type data fields
 */

void WorldPlot::_drawRangeRingsAndAzLines(QPainter &painter,
                                          bool ringsEnabled,
                                          bool azLinesEnabled,
                                          double ringSpacing)
{

  // save painter state

  painter.save();
  
  // Draw rings
  
  if (ringSpacing > 0.0 && ringsEnabled) {
    
    // Set up the painter
    
    painter.save();
    painter.setTransform(getTransform());
    painter.setPen(_params.grid_and_range_ring_color);
    
    // set narrow line width
    QPen pen = painter.pen();
    pen.setWidth(0);
    painter.setPen(pen);

    double ringRange = ringSpacing;
    while (ringRange <= _params.max_ring_range) {
      QRectF rect(-ringRange, -ringRange, ringRange * 2.0, ringRange * 2.0);
      painter.drawEllipse(rect);
      ringRange += ringSpacing;
    }
    painter.restore();

    // Draw the labels
    
    QFont font = painter.font();
    font.setPointSizeF(_params.range_ring_label_font_size);
    painter.setFont(font);
    // painter.setWindow(0, 0, width(), height());
    
    ringRange = ringSpacing;
    while (ringRange <= _params.max_ring_range) {
      double labelPos = ringRange * Constants::LUCID_SIN_45;
      const string &labelStr = _scaledLabel.scale(ringRange);
      drawText(painter, labelStr, labelPos, labelPos, Qt::AlignCenter);
      drawText(painter, labelStr, -labelPos, labelPos, Qt::AlignCenter);
      drawText(painter, labelStr, labelPos, -labelPos, Qt::AlignCenter);
      drawText(painter, labelStr, -labelPos, -labelPos, Qt::AlignCenter);
      ringRange += ringSpacing;
    }

  } /* endif - draw rings */

  // Draw the azimuth lines
  
  if (azLinesEnabled) {
    
    // Set up the painter
    
    painter.save();
    painter.setPen(_params.grid_and_range_ring_color);
  
    // Draw the lines along the X and Y axes

    drawLine(painter, 0, -_params.max_ring_range, 0, _params.max_ring_range);
    drawLine(painter, -_params.max_ring_range, 0, _params.max_ring_range, 0);

    // Draw the lines along the 30 degree lines

    double end_pos1 = Constants::LUCID_SIN_30 * _params.max_ring_range;
    double end_pos2 = Constants::LUCID_COS_30 * _params.max_ring_range;
    
    drawLine(painter, end_pos1, end_pos2, -end_pos1, -end_pos2);
    drawLine(painter, end_pos2, end_pos1, -end_pos2, -end_pos1);
    drawLine(painter, -end_pos1, end_pos2, end_pos1, -end_pos2);
    drawLine(painter, end_pos2, -end_pos1, -end_pos2, end_pos1);
    
    painter.restore();

  }

  painter.restore();

}
  


/////////////////////////////////////////////////////
// print

void WorldPlot::print(ostream &out)
  
{

  out << "================= WorldPlot properties ===================" << endl;

  out << "  _widthPixels        : " << _widthPixels << endl;
  out << "  _heightPixels       : " << _heightPixels << endl;
  out << "  _xPixOffset         : " << _xPixOffset << endl;
  out << "  _yPixOffset         : " << _yPixOffset << endl;
  out << "  _xMinWorld          : " << _xMinWorld << endl;
  out << "  _xMaxWorld          : " << _xMaxWorld << endl;
  out << "  _yMinWorld          : " << _yMinWorld << endl;
  out << "  _yMaxWorld          : " << _yMaxWorld << endl;
  out << "  _plotWidth          : " << _plotWidth << endl;
  out << "  _plotHeight         : " << _plotHeight << endl;
  out << "  _xMinPixel          : " << _xMinPixel << endl;
  out << "  _yMinPixel          : " << _yMinPixel << endl;
  out << "  _xMaxPixel          : " << _xMaxPixel << endl;
  out << "  _yMaxPixel          : " << _yMaxPixel << endl;
  out << "  _xPixelsPerWorld    : " << _xPixelsPerWorld << endl;
  out << "  _yPixelsPerWorld    : " << _yPixelsPerWorld << endl;
  out << "  _xMinWindow         : " << _xMinWindow << endl;
  out << "  _xMaxWindow         : " << _xMaxWindow << endl;
  out << "  _yMinWindow         : " << _yMinWindow << endl;
  out << "  _yMaxWindow         : " << _yMaxWindow << endl;

  out << "  _leftMargin         : " << _leftMargin << endl;
  out << "  _rightMargin        : " << _rightMargin << endl;
  out << "  _topMargin          : " << _topMargin << endl;
  out << "  _bottomMargin       : " << _bottomMargin << endl;
  out << "  _titleTextMargin    : " << _titleTextMargin << endl;
  out << "  _axisTextMargin     : " << _axisTextMargin << endl;
  out << "  _legendTextMargin   : " << _legendTextMargin << endl;
  out << "  _colorScaleWidth    : " << _colorScaleWidth << endl;
  out << "  _xAxisTickLen       : " << _xAxisTickLen << endl;
  out << "  _xNTicksIdeal       : " << _xNTicksIdeal << endl;
  out << "  _xSpecifyTicks      : " << _xSpecifyTicks << endl;
  out << "  _xTickMin           : " << _xTickMin << endl;
  out << "  _xTickDelta         : " << _xTickDelta << endl;
  out << "  _yAxisTickLen       : " << _yAxisTickLen << endl;
  out << "  _yNTicksIdeal       : " << _yNTicksIdeal << endl;
  out << "  _ySpecifyTicks      : " << _ySpecifyTicks << endl;
  out << "  _yTickMin           : " << _yTickMin << endl;
  out << "  _yTickDelta         : " << _yTickDelta << endl;
  out << "  _xAxisLabelsInside  : " << _xAxisLabelsInside << endl;
  out << "  _yAxisLabelsInside  : " << _yAxisLabelsInside << endl;
  out << "  _titleFontSize      : " << _titleFontSize << endl;
  out << "  _axisLabelFontSize  : " << _axisLabelFontSize << endl;
  out << "  _tickValuesFontSize : " << _tickValuesFontSize << endl;
  out << "  _legendFontSize     : " << _legendFontSize << endl;
  out << "  _titleColor         : " << _titleColor << endl;
  out << "  _axisLineColor      : " << _axisLineColor << endl;
  out << "  _axisTextColor      : " << _axisTextColor << endl;
  out << "  _gridColor          : " << _gridColor << endl;
  
  
  out << "==========================================================" << endl;

}
  
