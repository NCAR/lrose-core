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

#ifndef WorldPlot_HH
#define WorldPlot_HH

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QTransform>
#include <QColor>
#include <QFont>
#include <Radx/RadxTime.hh>
class ColorMap;

using namespace std;

class WorldPlot

{

public:

  // default constructor

  WorldPlot();
  
  // normal constructor
  
  WorldPlot(int widthPixels,
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
            int textMargin);

  // copy constructor
  
  WorldPlot(const WorldPlot &rhs);

  /// Assignment.
  
  WorldPlot& operator=(const WorldPlot &rhs);
  
  // set world view
  
  void set(int widthPixels,
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
           int textMargin);
  
  void set(double xMinWorld,
           double yMinWorld,
           double xMaxWorld,
           double yMaxWorld);

  // resize the plot
  
  void resize(int width, int height);

  // set the y scale from the x scale - for plotting with aspect ration 1.0

  void setYscaleFromXscale();
  
  // set methods
  
  inline void setAxisTickLen(int len) { _axisTickLen = len; }
  inline void setNTicksIdeal(int nTicks) { _nTicksIdeal = nTicks; }

  inline void setSpecifyTicks(bool specifyTicks,
                              double tickMin = 0.0,
                              double tickDelta = 0.0) {
    _specifyTicks = specifyTicks;
    _tickMin = tickMin;
    _tickDelta = tickDelta;
  }

  // get methods

  inline int getWidthPixels() const { return _widthPixels; }
  inline int getHeightPixels() const { return _heightPixels; }

  inline int getLeftMargin() const { return _leftMargin; }
  inline int getRightMargin() const { return _rightMargin; }
  inline int getTopMargin() const { return _topMargin; }
  inline int getBottomMargin() const { return _bottomMargin; }

  inline int getPlotWidth() const { return _plotWidth; }
  inline int getPlotHeight() const { return _plotHeight; }

  inline double getXMinWorld() const { return _xMinWorld; }
  inline double getXMaxWorld() const { return _xMaxWorld; }
  inline double getYMinWorld() const { return _yMinWorld; }
  inline double getYMaxWorld() const { return _yMaxWorld; }

  inline double getXPixelsPerWorld() const { return _xPixelsPerWorld; }
  inline double getYPixelsPerWorld() const { return _yPixelsPerWorld; }

  inline double getXMinWindow() const { return _xMinWindow; }
  inline double getXMaxWindow() const { return _xMaxWindow; }
  inline double getYMinWindow() const { return _yMinWindow; }
  inline double getYMaxWindow() const { return _yMaxWindow; }

  inline int getXMinPixel() const { return _xMinPixel; }
  inline int getYMinPixel() const { return _yMinPixel; }
  inline int getXMaxPixel() const { return _xMaxPixel; }
  inline int getYMaxPixel() const { return _yMaxPixel; }

  QRect getWorldWindow() const;
  QTransform getTransform() const { return _transform; }
  
  inline double getXPixel(double xWorld) const {
    return (xWorld - _xMinWorld) * _xPixelsPerWorld + _xMinPixel;
  }
  
  inline double getYPixel(double yWorld) const {
    return (yWorld - _yMinWorld) * _yPixelsPerWorld + _yMinPixel;
  }

  inline int getIxPixel(double xWorld) const {
    return (int) floor(getXPixel(xWorld) + 0.5);
  }
    
  inline int getIyPixel(double yWorld) const {
    return (int) floor(getYPixel(yWorld) + 0.5);
  }
  
  inline double getXWorld(double xPixel) const {
    return (xPixel - _xMinPixel) / _xPixelsPerWorld + _xMinWorld;
  }
  
  inline double getYWorld(double yPixel) const {
    return (yPixel - _yMinPixel) / _yPixelsPerWorld + _yMinWorld;
  }
  
  // draw a line in pixel coords
  
  void drawPixelLine(QPainter &painter,
                      double xx1, double yy1,
                      double xx2, double yy2);

  // draw a line in world coords

  void drawLine(QPainter &painter,
                double x1, double y1,
                double x2, double y2);

  // draw lines between consecutive points

  void drawLines(QPainter &painter, QVector<QPointF> &points);

  // draw a rectangle
  
  void drawRectangle(QPainter &painter,
                     double x, double y,
                     double w, double h);

  // fill a rectangle

  void fillRectangle(QPainter &painter,
                     QBrush &brush,
                     double x, double y,
                     double w, double h);

  // draw an arc

  void drawArc(QPainter &painter,
               double x, double y,
               double w, double h,
               double startAngle, double arcAngle);

  // draw a general path

  void drawPath(QPainter &painter, QPainterPath &path);

  // draw a general path clipped to within the margins

  void drawPathClipped(QPainter &painter, QPainterPath &path);

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
  
  void drawText(QPainter &painter, const string &text,
                double text_x, double text_y,
                int flags);
  
  void drawRotatedText(QPainter &painter, const string &text,
                       double text_x, double text_y,
                       int flags, double rotationDeg);
  
  void drawTextCentered(QPainter &painter, const string &text,
                        double text_x, double text_y);
	
  // Title
    
  void drawTitleTopCenter(QPainter &painter,
                          const string &title);
  // Y axis label
    
  void drawYAxisLabelLeft(QPainter &painter, const string &label);

  // Legends at top left
    
  void drawLegendsTopLeft(QPainter &painter, const vector<string> &legends);

  // Legends at top right
    
  void drawLegendsTopRight(QPainter &painter, const vector<string> &legends);

  // Legends at bottom left
    
  void drawLegendsBottomLeft(QPainter &painter, const vector<string> &legends);

  // Legends at bottom right
    
  void drawLegendsBottomRight(QPainter &painter, const vector<string> &legends);
	
  // left axis
    
  void drawAxisLeft(QPainter &painter, const string &units,
                    bool doLine, bool doTicks,
                    bool doLabels);
  // right axis
  
  void drawAxisRight(QPainter &painter, const string &units,
                     bool doLine, bool doTicks,
                     bool doLabels);
  // bottom axis
    
  void drawAxisBottom(QPainter &painter,
                      const string &units, bool doLine,
                      bool doTicks, bool doLabels);
  // top axis
  
  void drawAxisTop(QPainter &painter,
                   const string &units, bool doLine,
                   bool doTicks, bool doLabels);
  
  // draw range axes left and right, for bscan
  
  void drawRangeAxes(QPainter &painter,
                     const string &units,
                     bool drawGrid,
                     const QColor &lineColor,
                     const QColor &gridColor,
                     const QColor &textColor,
                     const QFont &labelFont,
                     const QFont &valuesFont,
                     bool unitsInFeet);
  
  // top and bottom axes as time variable, for bscan
  
  void drawTimeAxes(QPainter &painter,
                    const RadxTime &startTime,
                    const RadxTime &endTime,
                    bool drawGrid,
                    const QColor &lineColor,
                    const QColor &gridColor,
                    const QColor &textColor,
                    const QFont &labelFont,
                    const QFont &valuesFont,
                    bool drawDistTicks = false);
	
  // distance ticks on time axis

  void drawDistanceTicks(QPainter &painter,
                         const RadxTime &startTime,
                         const vector<double> &tickDists,
                         const vector<RadxTime> &tickTimes,
                         const QColor &lineColor,
                         const QColor &textColor,
                         const QFont &valuesFont);
  
  // draw axes bounding box
  
  void drawAxesBox(QPainter &painter);
  
  // compute linear ticks
  
  static vector<double> linearTicks(double minVal,
                                    double maxVal,
                                    int nTicksIdeal = 7,
                                    bool specifyTicks = false,
                                    double specifiedTickMin = 0.0,
                                    double specifiedTickDelta = 1.0);

  // get tick locations used for each axis, after drawing

  const vector<double> &getTopTicks() const { return _topTicks; }
  const vector<double> &getBottomTicks() const { return _bottomTicks; }
  const vector<double> &getLeftTicks() const { return _leftTicks; }
  const vector<double> &getRightTicks() const { return _rightTicks; }

  // draw source image into graphics, scaling and translating to
  // map the world coordinates
    
  void drawImage(QPainter &painter, QImage &image,
                 double xMinWorldImage, double xMaxWorldImage,
                 double yMinWorldImage, double yMaxWorldImage);

  // set clipping on to between margins

  void setClippingOn(QPainter &painter);
  
  // set clipping off

  void setClippingOff(QPainter &painter);

  // represent as string
  
  string asString();

  // get axis label given value and delta
    
  string getAxisLabel(double delta, double val);
    
  // draw the color scale

  void drawColorScale(const ColorMap &colorMap,
                      QPainter &painter,
                      int unitsFontSize);

protected:
private:

  // dimensions of the window in pixels

  int _widthPixels;
  int _heightPixels;

  // margins in pixels

  int _leftMargin;
  int _rightMargin;
  int _topMargin;
  int _bottomMargin;
  int _colorScaleWidth;

  // size of data area in pixels

  int _plotWidth;
  int _plotHeight;
  
  // world coord limits of data area

  double _xMinWorld;
  double _xMaxWorld;
  double _yMinWorld;
  double _yMaxWorld;

  // pixel coords of data area
  
  int _xMinPixel;
  int _yMinPixel;
  int _xMaxPixel;
  int _yMaxPixel;

  // scale in pixels per world coords

  double _xPixelsPerWorld;
  double _yPixelsPerWorld;

  // world coord limits of the window

  double _xMinWindow;
  double _xMaxWindow;
  double _yMinWindow;
  double _yMaxWindow;

  // axis plotting details

  int _axisTickLen;
  int _nTicksIdeal;
  int _textMargin;
  bool _specifyTicks;
  double _tickMin, _tickDelta;

  vector<double> _topTicks;
  vector<double> _bottomTicks;
  vector<double> _leftTicks;
  vector<double> _rightTicks;

  // affine transform

  QTransform _transform;
  void _computeTransform();

  // copy method for assignment and copy constructor

  WorldPlot & _copy(const WorldPlot &rhs);

};

#endif
