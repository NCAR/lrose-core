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

#ifndef WorldPlot_HH
#define WorldPlot_HH

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <QPainter>
#include <QRect>
#include <QTransform>
#include <QColor>
#include <QFont>
#include <Radx/RadxTime.hh>
#include <Mdv/MdvxProj.hh>
#include "cidd_macros.h"
#include "cidd_structs.h"
#include "cidd_colorscales.h"
class DsMdvxThreaded;
#include "cidd_field_data.h"

class ColorMap;

using namespace std;

class WorldPlot

{

public:
  
  // default constructor

  WorldPlot();
  
  // copy constructor
  
  WorldPlot(const WorldPlot &rhs);

  // Assignment.
  
  WorldPlot& operator=(const WorldPlot &rhs);
  
  // set size and location of plotting window within the main canvas
  // side effect - recomputes transform
  
  void setWindowGeom(int width,
                     int height,
                     int xOffset,
                     int yOffset);

  // set world coord limits for window
  // side effect - recomputes transform

  void setProjection(const MdvxProj &proj) { _proj = proj; }
     
  void setWorldLimits(double xMinWorld,
                      double yMinWorld,
                      double xMaxWorld,
                      double yMaxWorld);

  void setWorldLimitsX(double xMinWorld,
                       double xMaxWorld);

  void setWorldLimitsY(double yMinWorld,
                       double yMaxWorld);

  // set zoom limits from pixel space
  
  void setZoomLimits(int xMin,
                     int yMin,
                     int xMax,
                     int yMax);
  
  void setZoomLimitsX(int xMin,
                      int xMax);

  void setZoomLimitsY(int yMin,
                      int yMax);

  // set margins

  inline void setLeftMargin(int val) { _leftMargin = val; }
  inline void setRightMargin(int val) { _rightMargin = val; }
  inline void setTopMargin(int val) { _topMargin = val; }
  inline void setBottomMargin(int val) { _bottomMargin = val; }
  inline void setTitleTextMargin(int val) { _titleTextMargin = val; }
  inline void setAxisTextMargin(int val) { _axisTextMargin = val; }
  inline void setLegendTextMargin(int val) { _legendTextMargin = val; }

  // width of color scale
  
  void setColorScaleWidth(int val) { _colorScaleWidth = val; }

  // x ticks

  inline void setXAxisTickLen(int len) { _xAxisTickLen = len; }
  inline void setXNTicksIdeal(int nTicks) { _xNTicksIdeal = nTicks; }
  inline void specifyXTicks(double tickMin, double tickDelta) {
    _xSpecifyTicks = true;
    _xTickMin = tickMin;
    _xTickDelta = tickDelta;
  }
  inline void unspecifyXTicks() { _xSpecifyTicks = false; }

  // y ticks

  inline void setYAxisTickLen(int len) { _yAxisTickLen = len; }
  inline void setYNTicksIdeal(int nTicks) { _yNTicksIdeal = nTicks; }
  inline void specifyYTicks(double tickMin, double tickDelta) {
    _ySpecifyTicks = true;
    _yTickMin = tickMin;
    _yTickDelta = tickDelta;
  }
  inline void unspecifyYTicks() { _ySpecifyTicks = false; }

  // drawing titles, axes etc

  inline void setXAxisLabelsInside(bool val) { _xAxisLabelsInside = val; }
  inline void setYAxisLabelsInside(bool val) { _yAxisLabelsInside = val; }

  inline void setTitleFontSize(int val) { _titleFontSize = val; }
  inline void setAxisLabelFontSize(int val) { _axisLabelFontSize = val; }
  inline void setTickValuesFontSize(int val) { _tickValuesFontSize = val; }
  inline void setLegendFontSize(int val) { _legendFontSize = val; }

  inline void setBackgroundColor(const string &val) { _backgroundColor = val; }
  inline void setTitleColor(const string &val) { _titleColor = val; }
  inline void setAxisLineColor(const string &val) { _axisLineColor = val; }
  inline void setAxisTextColor(const string &val) { _axisTextColor = val; }
  inline void setGridColor(const string &val) { _gridColor = val; }

  // resize the plot
  
  void resize(int width, int height);

  // reset the offsets
  
  void setWindowOffsets(int xOffset, int yOffset);

  // set the y scale from the x scale - for plotting with aspect ration 1.0
  // or vice versa

  void setYscaleFromXscale();
  void setXscaleFromYscale();
  
  // get methods

  inline int getWidthPixels() const { return _widthPixels; }
  inline int getHeightPixels() const { return _heightPixels; }

  inline int getXPixOffset() const { return _xPixOffset; }
  inline int getYPixOffset() const { return _yPixOffset; }

  inline int getLeftMargin() const { return _leftMargin; }
  inline int getRightMargin() const { return _rightMargin; }
  inline int getTopMargin() const { return _topMargin; }
  inline int getBottomMargin() const { return _bottomMargin; }
  inline int getTitleTextMargin() const { return _titleTextMargin; }
  inline int getAxisTextMargin() const { return _axisTextMargin; }
  inline int getLegendTextMargin() const { return _legendTextMargin; }

  inline int getColorScaleWidth() const { return _colorScaleWidth; }

  inline const MdvxProj &getProjection() const { return _proj; }
     
  inline double getXMinWorld() const { return _xMinWorld; }
  inline double getXMaxWorld() const { return _xMaxWorld; }
  inline double getYMinWorld() const { return _yMinWorld; }
  inline double getYMaxWorld() const { return _yMaxWorld; }

  inline int getPlotWidth() const { return _plotWidth; }
  inline int getPlotHeight() const { return _plotHeight; }

  inline int getXMinPixel() const { return _xMinPixel; }
  inline int getYMinPixel() const { return _yMinPixel; }
  inline int getXMaxPixel() const { return _xMaxPixel; }
  inline int getYMaxPixel() const { return _yMaxPixel; }

  inline double getXPixelsPerWorld() const { return _xPixelsPerWorld; }
  inline double getYPixelsPerWorld() const { return _yPixelsPerWorld; }

  inline int getXPixCanvas(int xPix) const { return _xPixOffset + xPix; }
  inline int getYPixCanvas(int yPix) const { return _yPixOffset + yPix; }

  inline double getXMinWindow() const { return _xMinWindow; }
  inline double getXMaxWindow() const { return _xMaxWindow; }
  inline double getYMinWindow() const { return _yMinWindow; }
  inline double getYMaxWindow() const { return _yMaxWindow; }

  QRect getWorldWindow() const;
  QTransform getTransform() const { return _transform; }
  
  inline double getXPixel(double xWorld) const {
    return (xWorld - _xMinWorld) * _xPixelsPerWorld + _xMinPixel;
  }
  
  inline double getYPixel(double yWorld) const {
    return (yWorld - _yMinWorld) * _yPixelsPerWorld + _yMinPixel;
  }

  inline QPointF getPixelPointF(double xWorld, double yWorld) const {
    qreal xPixel = (xWorld - _xMinWorld) * _xPixelsPerWorld + _xMinPixel;
    qreal yPixel = (yWorld - _yMinWorld) * _yPixelsPerWorld + _yMinPixel;
    QPointF pt(xPixel, yPixel);
    return pt;
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
  
  // draw a point in pixel coords
  
  void drawPixelPoint(QPainter &painter,
                      double xx, double yy);

  // draw a point in world coords
  
  void drawPoint(QPainter &painter,
                 double xx, double yy);

  // draw points in pixel coords
  
  void drawPixelPoints(QPainter &painter,
                       const QVector<QPointF> &points);

  // draw points in world coords
  
  void drawPoints(QPainter &painter,
                  const QVector<QPointF> &points);

  // draw a line in pixel coords
  
  void drawPixelLine(QPainter &painter,
                     double xx1, double yy1,
                     double xx2, double yy2);

  // draw a line in world coords

  void drawLine(QPainter &painter,
                double x1, double y1,
                double x2, double y2);

  // draw lines between consecutive points

  void drawLines(QPainter &painter, const QVector<QPointF> &points);

  // draw a rectangle
  
  void drawRectangle(QPainter &painter,
                     double x, double y,
                     double w, double h);

  // fill a rectangle

  void fillRectangle(QPainter &painter,
                     const QBrush &brush,
                     double x, double y,
                     double w, double h);

  // fill the entire canvas with a color
  
  void fillCanvas(QPainter &painter,
                  const char *colorName);
  
  void fillCanvas(QPainter &painter,
                  const QBrush &brush);

  // fill a trapezium
  
  void fillTrap(QPainter &painter,
                const QBrush &brush,
                double x0, double y0,
                double x1, double y1,
                double x2, double y2,
                double x3, double y3);

  // draw an arc

  void drawArc(QPainter &painter,
               double x, double y,
               double w, double h,
               double startAngle, double arcAngle);

  // draw a general path

  void drawPath(QPainter &painter, QPainterPath &path);

  // draw a general path clipped to within the margins

  void drawPathClipped(QPainter &painter, QPainterPath &path);

  // draw a general path clipped to within the margins
  // points in screen coords
  
  void drawPathClippedScreenCoords(QPainter &painter,
                                   QPainterPath &path);

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

  // add text in world coords
  
  void drawText(QPainter &painter, const string &text,
                double text_x, double text_y,
                int flags);
  
  void drawRotatedText(QPainter &painter, const string &text,
                       double text_x, double text_y,
                       int flags, double rotationDeg);
  
  void drawTextCentered(QPainter &painter, const string &text,
                        double text_x, double text_y);
	
  // add text in screen coords
  
  void drawTextScreenCoords(QPainter &painter, const string &text,
                            int text_ix, int text_iy,
                            int flags);
  
  void drawRotatedTextScreenCoords(QPainter &painter, const string &text,
                                   int text_ix, int text_iy,
                                   int flags, double rotationDeg);
  
  void drawTextCenteredScreenCoords(QPainter &painter, const string &text,
                                    int text_ix, int text_iy);
	
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
    
  void drawAxisLeft(QPainter &painter,
                    const string &units,
                    bool doLine,
                    bool doTicks,
                    bool doLabels,
                    bool doGrid);
  // right axis
  
  void drawAxisRight(QPainter &painter,
                     const string &units,
                     bool doLine,
                     bool doTicks,
                     bool doLabels,
                     bool doGrid);
  // bottom axis
    
  void drawAxisBottom(QPainter &painter,
                      const string &units,
                      bool doLine,
                      bool doTicks,
                      bool doLabels,
                      bool doGrid);
  // top axis
  
  void drawAxisTop(QPainter &painter,
                   const string &units,
                   bool doLine,
                   bool doTicks,
                   bool doLabels,
                   bool doGrid);
  
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
  
  // adjust X and Y pixel scales to minimize distortion
  
  void adjustPixelScales();

  // render a data grid in Cartesian rectangular pixels

  void renderGridRect(int page,
                      QPainter &painter,
                      met_record_t *mr,
                      time_t start_time,
                      time_t end_time,
                      bool is_overlay_field);
  
  // render a data grid in distorted polygon

  void renderGridDistorted(int page,
                           QPainter &painter,
                           met_record_t *mr,
                           time_t start_time,
                           time_t end_time,
                           bool is_overlay_field);
  
  // print
  
  void print(ostream &out);

protected:
private:

  typedef struct {
    double x, y;
  } WorldPoint;

  // dimensions of the window in pixels
  
  int _widthPixels;
  int _heightPixels;

  // offset of the window in pixels
  // from the top-left of the main canvas

  int _xPixOffset;
  int _yPixOffset;

  // projection if applicable

  MdvxProj _proj;
  
  // world coord limits of data area

  double _xMinWorld;
  double _xMaxWorld;
  double _yMinWorld;
  double _yMaxWorld;

  // size of data area in pixels

  int _plotWidth;
  int _plotHeight;
  
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

  // margins in pixels

  int _leftMargin;
  int _rightMargin;
  int _topMargin;
  int _bottomMargin;
  int _titleTextMargin;
  int _axisTextMargin;
  int _legendTextMargin;

  // width for color scale if present

  int _colorScaleWidth;

  // axis ticks

  int _xAxisTickLen;
  int _xNTicksIdeal;
  bool _xSpecifyTicks;
  double _xTickMin;
  double _xTickDelta;

  int _yAxisTickLen;
  int _yNTicksIdeal;
  bool _ySpecifyTicks;
  double _yTickMin;
  double _yTickDelta;

  // control of axes, labels, grid

  bool _xAxisLabelsInside;
  bool _yAxisLabelsInside;

  int _titleFontSize;
  int _axisLabelFontSize;
  int _tickValuesFontSize;
  int _legendFontSize;

  string _backgroundColor;
  string _titleColor;
  string _axisLineColor;
  string _axisTextColor;
  string _gridColor;
  
  // axis ticks

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
