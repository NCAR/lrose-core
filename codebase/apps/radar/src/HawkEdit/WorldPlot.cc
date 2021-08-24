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
// World-coord plotting.
//
// Mike Dixon
//
// Developed in Java, Jan 2003
// Ported to C++, Oct 2014
//
////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <iostream>
#include <cstdio>
#include <QtCore/QLineF>
#include <toolsa/LogStream.hh>
#include <toolsa/toolsa_macros.h>
#include "WorldPlot.hh"
#include "ColorMap.hh"
using namespace std;

////////////////////////////////////////
// default constructor

WorldPlot::WorldPlot()
{
    
  set(100, 100,     // size
      0, 0, 0, 0,   // margins
      0,            // color scale width
      0.0, 0.0,     // min world
      100.0, 100.0, // max world
      7, 7, 5);     // axisTickLen, nTicksIdeal, textMargin
  
  _specifyTicks = false;
  _tickMin = 0.0;
  _tickDelta = 0.0;

}

////////////////////////////////////////
// normal constructor
  
WorldPlot::WorldPlot(int widthPixels,
                     int heightPixels,
                     int leftMargin,
                     int rightMargin,
                     int topMargin,
                     int bottomMargin,
                     int colorScaleWidth,
                     double xMinWorld,
                     double yMinWorld,
                     double xMaxWorld,
                     double yMaxWorld,
                     int axisTickLen,
                     int nTicksIdeal,
                     int textMargin)
{
    
  set(widthPixels,
      heightPixels,
      leftMargin,
      rightMargin,
      topMargin,
      bottomMargin,
      colorScaleWidth,
      xMinWorld,
      yMinWorld,
      xMaxWorld,
      yMaxWorld,
      axisTickLen,
      nTicksIdeal,
      textMargin);

  _specifyTicks = false;
  _tickMin = 0.0;
  _tickDelta = 0.0;

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
  
  _leftMargin = rhs._leftMargin;
  _rightMargin = rhs._rightMargin;
  _topMargin = rhs._topMargin;
  _bottomMargin = rhs._bottomMargin;
  _colorScaleWidth = rhs._colorScaleWidth;
  
  _plotWidth = rhs._plotWidth;
  _plotHeight = rhs._plotHeight;
  
  _xMinWorld = rhs._xMinWorld;
  _xMaxWorld = rhs._xMaxWorld;
  _yMinWorld = rhs._yMinWorld;
  _yMaxWorld = rhs._yMaxWorld;
  
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

  _axisTickLen = rhs._axisTickLen;
  _nTicksIdeal = rhs._nTicksIdeal;
  _textMargin = rhs._textMargin;

  _transform = rhs._transform;
  
  _specifyTicks = rhs._specifyTicks;
  _tickMin = rhs._tickMin;
  _tickDelta = rhs._tickDelta;

  return *this;

}

////////////////////////////////////////
// set world view
  
void WorldPlot::set(int widthPixels,
                    int heightPixels,
                    int leftMargin,
                    int rightMargin,
                    int topMargin,
                    int bottomMargin,
                    int colorScaleWidth,
                    double xMinWorld,
                    double yMinWorld,
                    double xMaxWorld,
                    double yMaxWorld,
                    int axisTickLen,
                    int nTicksIdeal,
                    int textMargin)
{
    
  _widthPixels = widthPixels;
  _heightPixels = heightPixels;
  
  _leftMargin = leftMargin;
  _rightMargin = rightMargin;
  _topMargin = topMargin;
  _bottomMargin = bottomMargin;
  _colorScaleWidth = colorScaleWidth;

  _xMinWorld = xMinWorld;
  _xMaxWorld = xMaxWorld;
  _yMinWorld = yMinWorld;
  _yMaxWorld = yMaxWorld;
  
  _axisTickLen = axisTickLen;
  _nTicksIdeal = nTicksIdeal;
  _textMargin = textMargin;
  
  _computeTransform();

}

void WorldPlot::set(double xMinWorld,
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

////////////////////////////////////////
// resize the plot

void WorldPlot::resize(int width, int height)

{
    
  _widthPixels = width;
  _heightPixels = height;
  
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
	
  double xx1 = getXPixel(x1);
  double yy1 = getYPixel(y1);
  double xx2 = xx1 + (x2 - x1) * _xPixelsPerWorld;
  double yy2 = yy1 + (y2 - y1) * _yPixelsPerWorld;

  QLineF qline(xx1, yy1, xx2, yy2);
  painter.drawLine(qline);

}

////////////////////////////////////////////////////////////////////////
// draw lines between consecutive points, world coords

void WorldPlot::drawLines(QPainter &painter, QVector<QPointF> &points) 

{

  QPainterPath path;

  double xx0 = getXPixel(points[0].x());
  double yy0 = getXPixel(points[0].y());

  path.moveTo(xx0, yy0);

  QVector<QLineF> lines;
    
  for (int ii = 1; ii < points.size(); ii++) {

    double xx = getXPixel(points[ii].x());
    double yy = getXPixel(points[ii].y());
      
    path.lineTo(xx, yy);

  }

  painter.drawPath(path);

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
  qreal yy;
  yy = (qreal) 2 * _textMargin;
  
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
    
  qreal xx = (qreal) tRect.height() / 2.0;
  qreal yy = (qreal) ((_yMinPixel + _yMaxPixel) / 2.0);

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

  qreal xx = (qreal) (_xMinPixel + _axisTickLen + _textMargin);
  qreal yy = _yMaxPixel + _axisTickLen;

  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy += (_textMargin + tRect.height());
  }

}

///////////////////////	
// Legends at top right
    
void WorldPlot::drawLegendsTopRight(QPainter &painter,
                                    const vector<string> &legends)

{

  qreal yy = _yMaxPixel + _axisTickLen;

  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    qreal xx = (qreal) (_xMaxPixel - _axisTickLen -
                        _textMargin - tRect.width());
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy += (_textMargin + tRect.height());
  }

}

//////////////////////////	
// Legends at bottom left
    
void WorldPlot::drawLegendsBottomLeft(QPainter &painter,
                                      const vector<string> &legends) 

{
	
  qreal xx = (qreal) (_xMinPixel + _axisTickLen + _textMargin);
  qreal yy = (qreal) (_yMinPixel - _axisTickLen);

  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    yy -= tRect.height();
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy -= _textMargin;
  }

}

///////////////////////////	
// Legends at bottom right
    
void WorldPlot::drawLegendsBottomRight(QPainter &painter,
                                       const vector<string> &legends) 

{
	
  qreal yy = (qreal) (_yMinPixel - _axisTickLen);

  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    qreal xx = (qreal) (_xMaxPixel - _axisTickLen -
                        _textMargin - tRect.width());
    yy -= tRect.height();
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy -= _textMargin;
  }

}

/////////////	
// left axis
    
void WorldPlot::drawAxisLeft(QPainter &painter, const string &units,
                             bool doLine, bool doTicks,
                             bool doLabels) 
  
{
	
  // axis line

  if (doLine) {
    drawLine(painter, _xMinWorld, _yMinWorld, _xMinWorld, _yMaxWorld);
  }

  // axis units label

  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX =
    (qreal) (_xMinPixel - unitsRect.width() - _textMargin);
  qreal unitsY = (qreal) (_yMaxPixel + (unitsRect.height() / 2));
  QRectF bRect(unitsX, unitsY, 
               unitsRect.width() + 2, unitsRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _leftTicks = linearTicks(_yMinWorld, _yMaxWorld,
                           _nTicksIdeal, _specifyTicks,
                           _tickMin, _tickDelta);

  if (_leftTicks.size() < 2) {
    return;
  }
	
  double delta = _leftTicks[1] - _leftTicks[0];
  for (size_t i = 0; i < _leftTicks.size(); i++) {
	    
    double val = _leftTicks[i];
    double pix = getYPixel(val);
    if (doTicks) {
      QLineF qline(_xMinPixel, pix,
                   _xMinPixel + _axisTickLen, pix);
      painter.drawLine(qline);
    }
	    
    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    qreal labelX = (qreal) (_xMinPixel - labelRect.width() - _textMargin);
    qreal labelY = (qreal) (pix - (labelRect.height() / 2));
    QRectF bRect2(labelX, labelY,
                  labelRect.width() + 2, labelRect.height() + 2);
    if ((fabs(labelY - unitsY) > labelRect.height() + _textMargin) &&
        (fabs(labelY - _yMinPixel) > labelRect.height() + _textMargin)) {
      if (doLabels) {
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }

  }
	
}

//////////////
// right axis
  
void WorldPlot::drawAxisRight(QPainter &painter, const string &units,
                              bool doLine, bool doTicks,
                              bool doLabels) 

{
	
  // axis line

  if (doLine) {
    drawLine(painter, _xMaxWorld, _yMinWorld, _xMaxWorld, _yMaxWorld);
  }

  // axis units label
	
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel + _textMargin);
  qreal unitsY = (qreal) (_yMaxPixel + (unitsRect.height() / 2));
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _rightTicks = linearTicks(_yMinWorld, _yMaxWorld,
                            _nTicksIdeal, _specifyTicks,
                            _tickMin, _tickDelta);
  if (_rightTicks.size() < 2) {
    return;
  }
	
  double delta = _rightTicks[1] - _rightTicks[0];
  for (size_t i = 0; i < _rightTicks.size(); i++) {
	    
    double val = _rightTicks[i];
    double pix = getYPixel(val);
    if (doTicks) {
      QLineF qline(_xMaxPixel, pix,
                   _xMaxPixel - _axisTickLen, pix);
      painter.drawLine(qline);
    }
	    
    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    qreal labelX = (qreal) (_xMaxPixel +_textMargin);
    qreal labelY = (qreal) (pix - (labelRect.height() / 2));
    QRectF bRect2(labelX, labelY,
                  labelRect.width() + 2, labelRect.height() + 2);

    if ((fabs(labelY - unitsY) > labelRect.height() + _textMargin) &&
        (fabs(labelY - _yMinPixel) > labelRect.height() + _textMargin)) {
      if (doLabels) {
        painter.drawText(bRect2, Qt::AlignCenter, label.c_str());
      }
    }
      
  }
	
} // drawAxisRight

///////////////  
// bottom axis
    
void WorldPlot::drawAxisBottom(QPainter &painter,
                               const string &units, bool doLine,
                               bool doTicks, bool doLabels) 

{
	
  // axis line
  
  if (doLine) {
    drawLine(painter, _xMinWorld, _yMinWorld, _xMaxWorld, _yMinWorld);
  }
    
  // axis units label
	
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel - unitsRect.width() / 2);
  qreal unitsY =
    (qreal) (_yMinPixel + (unitsRect.height() + _textMargin));
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _bottomTicks = linearTicks(_xMinWorld, _xMaxWorld,
                             _nTicksIdeal, _specifyTicks,
                             _tickMin, _tickDelta);
  if (_bottomTicks.size() < 2) {
    return;
  }
	
  double delta = _bottomTicks[1] - _bottomTicks[0];
  for (size_t i = 0; i < _bottomTicks.size(); i++) {
	    
    double val = _bottomTicks[i];
    double pix = getXPixel(val);
    if (doTicks) {
      QLineF qline(pix, _yMinPixel,
                   pix, _yMinPixel - _axisTickLen);
      painter.drawLine(qline);
    }
	    
    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().tightBoundingRect(label.c_str()));
    if (((pix + labelRect.width() / 2 + _textMargin) < unitsX) &&
        ((pix - labelRect.width() / 2 - _textMargin) > _xMinPixel)) {
      qreal labelX = (qreal) (pix - labelRect.width() / 2.0);
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
                            const string &units, bool doLine,
                            bool doTicks, bool doLabels)

{
	
  // axis line
	
  if (doLine) {
    drawLine(painter, _xMinWorld, _yMaxWorld, _xMaxWorld, _yMaxWorld);
  }
	
  // axis units label
	
  QRect unitsRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
  qreal unitsX = (qreal) (_xMaxPixel - unitsRect.width() / 2);
  qreal unitsY = (qreal) (_yMaxPixel - (unitsRect.height() + _textMargin) - 2);
  QRectF bRect(unitsX, unitsY,
               unitsRect.width() + 2, unitsRect.height() + 2);
  if (doLabels) {
    painter.drawText(bRect, Qt::AlignCenter, units.c_str());
  }

  // tick marks

  _topTicks = linearTicks(_xMinWorld, _xMaxWorld,
                          _nTicksIdeal, _specifyTicks,
                          _tickMin, _tickDelta);
  if (_topTicks.size() < 2) {
    return;
  }
  
  double delta = _topTicks[1] - _topTicks[0];
  for (size_t i = 0; i < _topTicks.size(); i++) {
	    
    double val = _topTicks[i];
    double pix = getXPixel(val);
    if (doTicks) {
      QLineF qline(pix, _yMaxPixel,
                   pix, _yMaxPixel + _axisTickLen);
      painter.drawLine(qline);
    }
    
    string label(getAxisLabel(delta, val));
    QRect labelRect(painter.fontMetrics().boundingRect(label.c_str()));
    if (((pix + labelRect.width() / 2 + _textMargin) < unitsX) &&
        ((pix - labelRect.width() / 2 - _textMargin) > _xMinPixel)) {
      qreal labelX = (qreal) (pix - labelRect.width() / 2.0);
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
  drawLine(painter, _xMinWorld, _yMaxWorld, _xMinWorld, _yMaxWorld);

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
    (qreal) (_xMinPixel - unitsRect.width() - _textMargin);
  qreal unitsY = (qreal) (_yMaxPixel + (unitsRect.height() / 2));
  QRectF bRectLeft(unitsXLeft, unitsY, 
                   unitsRect.width() + 2, unitsRect.height() + 2);
  painter.drawText(bRectLeft, Qt::AlignCenter, units.c_str());

  qreal unitsXRight = (qreal) (_xMaxPixel + _textMargin);
  QRectF bRectRight(unitsXRight, unitsY,
                    unitsRect.width() + 2, unitsRect.height() + 2);
  painter.drawText(bRectRight, Qt::AlignCenter, units.c_str());

  // tick marks

  vector<double> ticks = linearTicks(_yMinWorld * unitsMult,
                                     _yMaxWorld * unitsMult,
                                     _nTicksIdeal, _specifyTicks,
                                     _tickMin, _tickDelta);
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
                     _xMinPixel + _axisTickLen, pix);
    painter.drawLine(qlineLeft);
    
    // ticks right
    
    QLineF qlineRight(_xMaxPixel, pix,
                      _xMaxPixel - _axisTickLen, pix);
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
    qreal labelXLeft = (qreal) (_xMinPixel - labelRect.width() - _textMargin);
    qreal labelXRight = (qreal) (_xMaxPixel + _textMargin);
    qreal labelY = (qreal) (pix - (labelRect.height() / 2));
    QRectF bRect2Left(labelXLeft, labelY,
                      labelRect.width() + 2, labelRect.height() + 2);
    QRectF bRect2Right(labelXRight, labelY,
                       labelRect.width() + 2, labelRect.height() + 2);
    if ((fabs(labelY - unitsY) > labelRect.height() + _textMargin) &&
        (fabs(labelY - _yMinPixel) > labelRect.height() + _textMargin)) {
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
    (qreal) (_yMinPixel + (unitsRect.height() + _textMargin));
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
    QLineF qlineBottom(pix, _yMinPixel, pix, _yMinPixel - _axisTickLen);
    painter.drawLine(qlineBottom);
    QLineF qlineTop(pix, _yMaxPixel, pix, _yMaxPixel + _axisTickLen);
    painter.drawLine(qlineTop);

    // time labels - bottom axis
    
    painter.setPen(textColor);
    painter.setFont(valuesFont);

    char timeLabel[1024];
    sprintf(timeLabel, "%.2d:%.2d:%.2d",
            tickTime.getHour(), tickTime.getMin(), tickTime.getSec());
    QRect labelRect(painter.fontMetrics().tightBoundingRect(timeLabel));
    if (((pix + labelRect.width() / 2 + _textMargin) < unitsX) &&
        ((pix - labelRect.width() / 2 - _textMargin) > _xMinPixel)) {
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
    
    if (!isfinite(distVal)) {
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
      
      QLineF qline(pix, _yMinPixel, pix, _yMinPixel - _axisTickLen);
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
                          double yMinWorldImage, double yMaxWorldImage) {
	
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
    
  _xMinPixel = _leftMargin;
  _xMaxPixel = _xMinPixel + _plotWidth - 1;
  // OR ...
  //_yMinPixel = _topMargin + _plotHeight - 1;
  //_yMaxPixel = _topMargin;

  _yMinPixel = _topMargin; 
  _yMaxPixel = _yMinPixel + _plotHeight - 1;
 
    
  _xPixelsPerWorld =
    abs((_xMaxPixel - _xMinPixel) / (_xMaxWorld - _xMinWorld));
  _yPixelsPerWorld =
    abs((_yMaxPixel - _yMinPixel) / (_yMaxWorld - _yMinWorld));
    
  _transform.reset();
  int centerXPixelSpace = _xMinPixel + _plotWidth/2;
  //int centerYPixelSpace = _yMinPixel - _plotHeight/2;
  // OR ...
  int centerYPixelSpace = _yMinPixel + _plotHeight/2;  

  _transform.translate(centerXPixelSpace, centerYPixelSpace);
  LOG(DEBUG) << "translating to x,y in pixel space " << 
    centerXPixelSpace << ", " << centerYPixelSpace;
  //_transform.translate(_xMinPixel, _yMinPixel);
  //_transform.scale(_xPixelsPerWorld, _yPixelsPerWorld);
  // 
  // OR ...

  //_transform.translate(-_xMinWorld, -_yMinWorld); 
  _transform.scale(_xPixelsPerWorld, _yPixelsPerWorld);
  _transform.rotateRadians(M_PI, Qt::XAxis);

    
  _xMinWindow = getXWorld(0);
  _yMinWindow = getYWorld(0);
  _xMaxWindow = getXWorld(_widthPixels);
  _yMaxWindow = getYWorld(_heightPixels);

}

/////////////////////////////////////////////////////
// draw the color scale

void WorldPlot::drawColorScale(const ColorMap &colorMap,
                               QPainter &painter,
                               int unitsFontSize)
  
{
  
  const std::vector<ColorMap::CmapEntry> &cmap = colorMap.getEntries();

  int pltHt = _plotHeight;
  int width = _colorScaleWidth;
  int xStart = _widthPixels - width;
  size_t nHts = cmap.size() + 1; // leave some space at top and bottom
  double patchHt = (double)(pltHt) / nHts;
  int iPatchHt = (int) patchHt;

  // fill the swatches with the color
  
  painter.save();
  painter.setPen(Qt::SolidLine);
  int scaleYTop = 0, scaleYBot = 0;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    QColor color(entry.red, entry.green, entry.blue);
    painter.setBrush(color);
    double topY = pltHt - (int) (ii + 2) * patchHt + (patchHt / 2) + _topMargin;
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
    
    double yy = pltHt - (patchHt * 1.0) + _topMargin;
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

  // add Units label

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

