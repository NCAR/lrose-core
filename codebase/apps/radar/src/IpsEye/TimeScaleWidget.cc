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
#include <assert.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg.h>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "TimeScaleWidget.hh"
#include "PolarManager.hh"

using namespace std;

TimeScaleWidget::TimeScaleWidget(QWidget* parent,
                                 const PolarManager &manager,
                                 const Params &params) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(params),
        _backgroundBrush(QColor(_params.background_color)),
        _scaledLabel(ScaledLabel::DistanceEng),
        _worldReleaseX(0),
        _worldReleaseY(0)

{

  _startTime.set(0);  
  _endTime.set(1);
  _timeSpanSecs = 1;
  _timesPending = false;
  _pointClicked = false;
  
  // Set up the background color

  QPalette new_palette = palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  setPalette(new_palette);
  
  setBackgroundRole(QPalette::Dark);
  setAutoFillBackground(true);
  setAttribute(Qt::WA_OpaquePaintEvent);
  
  // Allow the widget to get focus
  
  setFocusPolicy(Qt::StrongFocus);
  
  // Allow the size_t type to be passed to slots
  
  // qRegisterMetaType<size_t>("size_t");

  setFixedHeight(100);

}

/*************************************************************************
 * Destructor
 */

TimeScaleWidget::~TimeScaleWidget()
{

}


//////////////////////////////////
// set the plot times

void TimeScaleWidget::setTimes(const RadxTime &startTime,
                               const RadxTime &endTime)
{
  
  _startTime = startTime;
  _endTime = endTime;
  _timeSpanSecs = _endTime - _startTime;
  _pointClicked = false;
  _timesPending = true;

}

/*************************************************************************
 * configure the axes
 */

void TimeScaleWidget::configureAxes()

{

  // set bottom margin - increase this if we are plotting the distance labels and ticks
  
  int bottomMargin = 40;
  int topMargin = 40;
  int leftMargin = 40;
  int rightMargin = 40;
  
  _world.setWindowGeom(width(), height(), 0, 0);
  _world.setLeftMargin(leftMargin);
  _world.setRightMargin(rightMargin);
  _world.setTopMargin(topMargin);
  _world.setBottomMargin(bottomMargin);
  _world.setColorScaleWidth(0.0);
  _world.setWorldLimits(_startTime.asDouble(), 0.0, _endTime.asDouble(), 1.0);
  _world.setXAxisTickLen(_params.bscan_axis_tick_len);
  _world.setXNTicksIdeal(_params.bscan_n_ticks_ideal);
  _world.setAxisTextMargin(_params.bscan_text_margin);
    
  _transform = _world.getTransform();

}

/*************************************************************************
 * refresh()
 */

void TimeScaleWidget::refresh()
{
  _refresh();
}

void TimeScaleWidget::_refresh()
{
  if (_timesPending) {
    configureAxes();
    _timesPending = false;
  }
  QPainter painter(this);
  QPaintDevice *device = painter.device();
  if (device == 0) {
    return;
  }
  _drawOverlays(painter);
}

/*************************************************************************
 * backgroundColor()
 */

void TimeScaleWidget::setBackgroundColor(const QColor &color)
{
  
  _backgroundBrush.setColor(color);
  
  QPalette new_palette = palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  setPalette(new_palette);
  
  // _refresh();

}


/*************************************************************************
 * Slots
 *************************************************************************/

/*************************************************************************
 * mousePressEvent()
 */

void TimeScaleWidget::mousePressEvent(QMouseEvent *e)
{

  _mousePressX = e->position().x();
  _mousePressY = e->position().y();
  
  _worldPressX = _world.getXWorld(_mousePressX);
  _worldPressY = _world.getYWorld(_mousePressY);

}


/*************************************************************************
 * mouseReleaseEvent()
 */

void TimeScaleWidget::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  QPointF clickPos(e->pos());
  _mouseReleaseX = clickPos.x();
  _mouseReleaseY = clickPos.y();

  // get click location in world coords

  _worldReleaseX = _world.getXWorld(_mouseReleaseX);
  _worldReleaseY = _world.getYWorld(_mouseReleaseY);

  // Emit a signal to indicate that the click location has changed
  
  _pointClicked = true;
  
  update();

}


/*************************************************************************
 * paintEvent()
 */

void TimeScaleWidget::paintEvent(QPaintEvent *event)
{

  // RadxTime now(RadxTime::NOW);
  // double timeSinceLast = now - _timeLastRendered;
  // if (timeSinceLast < _params.bscan_min_secs_between_rendering_beams) {
  //   return;
  // }
  // _timeLastRendered = now;

  QPainter painter(this);
  QPaintDevice *device = painter.device();
  if (device == 0) {
    return;
  }
  painter.save();
  painter.eraseRect(0, 0, width(), height());
  _world.setClippingOn(painter);
  painter.restore();
  _drawOverlays(painter);

}


/*************************************************************************
 * resizeEvent()
 */

void TimeScaleWidget::resizeEvent(QResizeEvent * e)
{

  _resetWorld(width(), height());
  configureAxes();

  if (_timesPending) {
    _timesPending = false;
  }

  QPainter painter(this);
  QPaintDevice *device = painter.device();
  if (device == 0) {
    return;
  }
  _drawOverlays(painter);
  update();
  
}


/*************************************************************************
 * resize()
 */

void TimeScaleWidget::resize(int width, int height)
{

  setGeometry(0, 0, width, height);
  _resetWorld(width, height);
  
}

//////////////////////////////////////////////////////////////
// reset the pixel size of the world view

void TimeScaleWidget::_resetWorld(int width, int height)

{

  _world.resize(width, height);
  _transform = _world.getTransform();

}

/*************************************************************************
 * set mouse click point from external routine, to simulate and actual
 * mouse release event
 */

void TimeScaleWidget::setMouseClickPoint(double worldX,
                                         double worldY)
{

  if (_pointClicked) {

    _worldReleaseX = worldX;
    _worldReleaseY = worldY;
    
    _mouseReleaseX = _world.getIxPixel(_worldReleaseX);
    _mouseReleaseY = _world.getIyPixel(_worldReleaseY);
    
    // Update the window
  
    update();

  }

}


/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * Draw the overlays, axes, legends etc
 */

void TimeScaleWidget::_drawOverlays(QPainter &painter)
{

  QPaintDevice *device = painter.device();
  if (device == 0) {
    return;
  }

  // save painter state
  
  painter.save();

  // store font
  
  QFont origFont = painter.font();
  
  // Set the painter to use the right color and font

  painter.setPen(_params.bscan_axes_color);
  
  // axes and labels

  QFont font(origFont);
  font.setPointSizeF(_params.bscan_axis_label_font_size);
  painter.setFont(font);
  // painter.setWindow(0, 0, width(), height());

  // axes

  QColor lineColor(_params.bscan_axes_color);
  QColor gridColor(_params.bscan_grid_color);
  QColor textColor(_params.bscan_labels_color);

  QFont labelFont(origFont);
  labelFont.setPointSizeF(_params.bscan_axis_label_font_size);
  QFont valuesFont(origFont);
  valuesFont.setPointSizeF(_params.bscan_axis_values_font_size);
  
  _world.drawTimeAxes(painter,
                      _startTime, _endTime, false,
                      lineColor, gridColor, textColor,
                      labelFont, valuesFont, false);
  
  // title
  
  font.setPointSizeF(_params.bscan_title_font_size);
  painter.setFont(font);
  _world.drawTitleTopCenter(painter, "Time selector");

  // click point cross hairs
  
  if (_pointClicked) {

    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;
    
    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

  }

  // reset painter state
  
  painter.restore();

}

