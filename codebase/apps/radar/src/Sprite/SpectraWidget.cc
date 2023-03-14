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
#include <toolsa/DateTime.hh>
#include <toolsa/pjg.h>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPen>
#include <QResizeEvent>
#include <QInputDialog>

#include "SpectraWidget.hh"
#include "SpectraMgr.hh"
#include "Beam.hh"
#include "AscopePlot.hh"
#include "WaterfallPlot.hh"
#include "IqPlot.hh"

using namespace std;

SpectraWidget::SpectraWidget(QWidget* parent,
                             const SpectraMgr &manager,
                             const Params &params) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(params),
        _backgroundBrush(QColor(_params.main_background_color)),
        _scaledLabel(ScaledLabel::DistanceEng),
        _worldReleaseX(0),
        _worldReleaseY(0),
        _rubberBand(0),
        _beam(NULL),
        _nSamplesPlot(0)

{

  // init

  _pointClicked = false;

  _titleMargin = _params.main_window_title_margin;

  _ascopeStartIx = 0;
  _nAscopes = _params.ascope_n_panels;
  _ascopeWidth = _params.ascope_width; // constant
  _ascopeHeight = 100;
  _ascopeGrossWidth = _ascopeWidth * _nAscopes;

  _nWaterfalls = _params.waterfall_n_panels;
  _waterfallStartIx = _ascopeGrossWidth;
  _waterfallWidth = _params.waterfall_width; // constant
  _waterfallHeight = 100;
  _waterfallGrossWidth = _waterfallWidth * _nWaterfalls;

  _iqStartIx = _ascopeGrossWidth + _waterfallGrossWidth;
  _nIqRows = _params.iqplots_n_rows;
  _nIqCols = _params.iqplots_n_columns;
  _nIqPlots = _nIqRows * _nIqCols;
  
  _iqGrossHeight = height() - _titleMargin;
  _iqGrossWidth = width() - _iqStartIx;
  _iqPlotWidth = _iqGrossWidth / _nIqCols;
  _iqPlotHeight = _iqGrossHeight / _nIqRows;

  _ascopesConfigured = false;
  _waterfallsConfigured = false;
  _iqPlotsConfigured = false;

  _selectedRangeKm = _params.start_range_km;

  // Set up the background color

  QPalette new_palette = palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  setPalette(new_palette);
  
  setBackgroundRole(QPalette::Dark);
  setAutoFillBackground(true);
  setAttribute(Qt::WA_OpaquePaintEvent);
  
  // Allow the widget to get focus
  
  setFocusPolicy(Qt::StrongFocus);
  
  // create the rubber band

  _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

  // Allow the size_t type to be passed to slots
  
  qRegisterMetaType<size_t>("size_t");

  // set up world views
  
  configureAxes(0.0, 1.0,
                _params.archive_time_span_secs);

  // create ascopes

  for (int ii = 0; ii < _nAscopes; ii++) {
    _createAscope(ii);
  }

  // create waterfalls

  for (int ii = 0; ii < _nWaterfalls; ii++) {
    _createWaterfall(ii);
  }

  // create iqPlots

  for (int ii = 0; ii < _nIqPlots; ii++) {
    _createIqPlot(ii);
  }

  // set up context menus

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), 
          this, SLOT(showContextMenu(const QPoint &)));

}

/*************************************************************************
 * Destructor
 */

SpectraWidget::~SpectraWidget()
{

  for (size_t ii = 0; ii < _ascopes.size(); ii++) {
    delete _ascopes[ii];
  }
  _ascopes.clear();

  for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
    delete _waterfalls[ii];
  }
  _waterfalls.clear();

  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
    delete _iqPlots[ii];
  }
  _iqPlots.clear();

}


/*************************************************************************
 * configure the axes
 */

void SpectraWidget::configureAxes(double min_amplitude,
                                  double max_amplitude,
                                  double time_span_secs)
  
{

  _minAmplitude = min_amplitude;
  _maxAmplitude = max_amplitude;
  _timeSpanSecs = time_span_secs;
  _plotEndTime = _plotStartTime + _timeSpanSecs;
  
  // set bottom margin - increase this if we are plotting the distance labels and ticks
  
  _fullWorld.setWindowGeom(width() / 3, height() / 3, 0, 0);

  _fullWorld.setWorldLimits(0.0, _minAmplitude,
                            _timeSpanSecs, _maxAmplitude);

  _fullWorld.setLeftMargin(_params.iqplot_left_margin);
  _fullWorld.setRightMargin(_params.iqplot_right_margin);
  _fullWorld.setTopMargin(_params.iqplot_top_margin);
  _fullWorld.setBottomMargin(_params.iqplot_bottom_margin);
  _fullWorld.setTitleTextMargin(_params.iqplot_title_text_margin);
  _fullWorld.setLegendTextMargin(_params.iqplot_legend_text_margin);
  _fullWorld.setAxisTextMargin(_params.iqplot_axis_text_margin);

  _fullWorld.setColorScaleWidth(0);

  _fullWorld.setXAxisTickLen(_params.iqplot_axis_tick_len);
  _fullWorld.setXNTicksIdeal(_params.iqplot_n_ticks_ideal);
  _fullWorld.setYAxisTickLen(_params.iqplot_axis_tick_len);
  _fullWorld.setYNTicksIdeal(_params.iqplot_n_ticks_ideal);

  _fullWorld.setTitleFontSize(_params.iqplot_title_font_size);
  _fullWorld.setAxisLabelFontSize(_params.iqplot_axis_label_font_size);
  _fullWorld.setTickValuesFontSize(_params.iqplot_tick_values_font_size);
  _fullWorld.setLegendFontSize(_params.iqplot_legend_font_size);

  _fullWorld.setTitleColor(_params.iqplot_title_color);
  _fullWorld.setAxisLineColor(_params.iqplot_axes_color);
  _fullWorld.setAxisTextColor(_params.iqplot_axes_color);
  _fullWorld.setGridColor(_params.iqplot_grid_color);

  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());

  // refresh all paints

  _refresh();

}

/*************************************************************************
 * clear()
 */

void SpectraWidget::clear()
{

  // Clear out the beam array
  
  _pointClicked = false;
  
  // Now rerender the images
  
  _refresh();
  
}


/*************************************************************************
 * refresh()
 */

void SpectraWidget::refresh()
{
  _refresh();
}

/*************************************************************************
 * unzoom the view
 */

void SpectraWidget::unzoom()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());

  for (size_t ii = 0; ii < _ascopes.size(); ii++) {
    _ascopes[ii]->unzoom();
  }

  for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
    _waterfalls[ii]->unzoom();
  }

  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
    _iqPlots[ii]->unzoom();
  }

  _refresh();

}

/*************************************************************************
 * setGrids()
 */

void SpectraWidget::setXGridEnabled(bool state)
{
  _xGridEnabled = state;
  for (size_t ii = 0; ii < _ascopes.size(); ii++) {
    _ascopes[ii]->setXGridLinesOn(state);
  }
  for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
    _waterfalls[ii]->setXGridLinesOn(state);
  }
  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
    _iqPlots[ii]->setXGridLinesOn(state);
  }
  update();
}

void SpectraWidget::setYGridEnabled(bool state)
{
  _yGridEnabled = state;
  for (size_t ii = 0; ii < _ascopes.size(); ii++) {
    _ascopes[ii]->setYGridLinesOn(state);
  }
  for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
    _waterfalls[ii]->setYGridLinesOn(state);
  }
  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
    _iqPlots[ii]->setYGridLinesOn(state);
  }
  update();
}

void SpectraWidget::setLegendsEnabled(bool state)
{
  _legendsEnabled = state;
  for (size_t ii = 0; ii < _ascopes.size(); ii++) {
    _ascopes[ii]->setLegendsOn(state);
  }
  for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
    _waterfalls[ii]->setLegendsOn(state);
  }
  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
    _iqPlots[ii]->setLegendsOn(state);
  }
  update();
}

/*************************************************************************
 * plot a beam
 */

void SpectraWidget::plotBeam(Beam *beam)

{

  if(_params.debug) {
    cerr << "======== SpectraWidget handling beam ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
  }

  _beam = beam;
  _nSamplesPlot = _beam->getNSamples();
  iwrf_xmit_rcv_mode_t xmitRcvMode = _beam->getXmitRcvMode();
  if (xmitRcvMode == IWRF_ALT_HV_CO_CROSS ||
      xmitRcvMode == IWRF_ALT_HV_CO_ONLY ||
      xmitRcvMode == IWRF_ALT_HV_FIXED_HV) {
    _nSamplesPlot /= 2; // alternating mode
  }


  if (_ascopes.size() > 0 && !_ascopesConfigured) {
    for (size_t ii = 0; ii < _ascopes.size(); ii++) {
      _configureAscope(ii);
    }
    _ascopesConfigured = true;
  }
  
  if (_waterfalls.size() > 0 && !_waterfallsConfigured) {
    for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
      _configureWaterfall(ii);
    }
    _waterfallsConfigured = true;
  }
  
  if (_iqPlots.size() > 0 && !_iqPlotsConfigured) {
    for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
      _configureIqPlot(ii);
    }
    _iqPlotsConfigured = true;
  }
  
  update();

}

/*************************************************************************
 * backgroundColor()
 */

void SpectraWidget::setBackgroundColor(const QColor &color)
{

  _backgroundBrush.setColor(color);
  
  QPalette new_palette = palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  setPalette(new_palette);
  
  _refresh();

}


/*************************************************************************
 * getImage()
 */

QImage* SpectraWidget::getImage()
{

  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;

}


/*************************************************************************
 * getPixmap()
 */

QPixmap* SpectraWidget::getPixmap()
{

  QPixmap* pixmap = new QPixmap(grab());
  return pixmap;

}


/*************************************************************************
 * Slots
 *************************************************************************/

/*************************************************************************
 * mousePressEvent()
 */

void SpectraWidget::mousePressEvent(QMouseEvent *e)
{

  _rubberBand->setGeometry(QRect(e->pos(), QSize()));
  _rubberBand->show();

  _mousePressX = e->x();
  _mousePressY = e->y();

  _worldPressX = _zoomWorld.getXWorld(_mousePressX);
  _worldPressY = _zoomWorld.getYWorld(_mousePressY);

  // save the panel details for the mouse press point

  _identSelectedPanel(_mousePressX, _mousePressY,
                      _mousePressPanelType, _mousePressPanelId);

}


/*************************************************************************
 * mouseMoveEvent()
 */

void SpectraWidget::mouseMoveEvent(QMouseEvent * e)
{
  // Zooming with the mouse

  int x = e->x();
  int y = e->y();
  int deltaX = x - _mousePressX;
  int deltaY = y - _mousePressY;

  QRect newRect = QRect(_mousePressX, _mousePressY, (int) deltaX, (int) deltaY);

  newRect = newRect.normalized();
  _rubberBand->setGeometry(newRect);

}


/*************************************************************************
 * mouseReleaseEvent()
 */

void SpectraWidget::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  QRect rgeom = _rubberBand->geometry();

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

  QPointF clickPos(e->pos());
  
  _mouseReleaseX = clickPos.x();
  _mouseReleaseY = clickPos.y();
  
  // save the panel details for the mouse release point

  _identSelectedPanel(_mouseReleaseX,
                      _mouseReleaseY,
                      _mouseReleasePanelType,
                      _mouseReleasePanelId);

  // get click location in world coords

  _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
  _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

  if (rgeom.width() <= 20) {
    
    if (_mousePressPanelType == PANEL_ASCOPE &&
        _mouseReleasePanelType == PANEL_ASCOPE &&
        _mousePressPanelId == _mouseReleasePanelId) {
      // change ascope range
      AscopePlot *ascope = _ascopes[_mouseReleasePanelId];
      _selectedRangeKm = ascope->getZoomWorld().getYWorld(_mouseReleaseY);
      if (_beam != NULL) {
        _selectedRangeKm =
          _beam->getClosestRange(_selectedRangeKm, _selectedGateNum);
        emit locationClicked(_selectedRangeKm, _selectedGateNum);
      }
    } else if (_mousePressPanelType == PANEL_WATERFALL &&
               _mouseReleasePanelType == PANEL_WATERFALL &&
               _mousePressPanelId == _mouseReleasePanelId) {
      // change waterfall range
      WaterfallPlot *waterfall = _waterfalls[_mouseReleasePanelId];
      _selectedRangeKm = waterfall->getZoomWorld().getYWorld(_mouseReleaseY);
      if (_beam != NULL) {
        _selectedRangeKm =
          _beam->getClosestRange(_selectedRangeKm, _selectedGateNum);
        emit locationClicked(_selectedRangeKm, _selectedGateNum);
      }
    } else {
      _pointClicked = true;
    }
    
  } else {

    // mouse moved more than 20 pixels, so a zoom occurred
    
    if (_mousePressPanelType == PANEL_ASCOPE &&
        _mouseReleasePanelType == PANEL_ASCOPE) {
      // ascope zoom
      if (_mousePressPanelId == _mouseReleasePanelId) {
        // zoom in one panel, reflect the range change in 
        // other panels
        for (int ii = 0; ii < (int) _ascopes.size(); ii++) {
          AscopePlot *ascope = _ascopes[ii];
          if (ii == _mouseReleasePanelId) {
            // perform 2D zoom
            ascope->setZoomLimits(_mousePressX, _mousePressY,
                                  _mouseReleaseX, _mouseReleaseY);
          } else {
            // perform zoom in range only
            ascope->setZoomLimitsY(_mousePressY, _mouseReleaseY);
          }
        } // ii
      } else {
        // zoom box crosses panels, zoom in range only
        for (int ii = 0; ii < (int) _ascopes.size(); ii++) {
          AscopePlot *ascope = _ascopes[ii];
          ascope->setZoomLimitsY(_mousePressY, _mouseReleaseY);
        }
      }

    } else if (_mousePressPanelType == PANEL_WATERFALL &&
               _mouseReleasePanelType == PANEL_WATERFALL) {
      // waterfall zoom
      if (_mousePressPanelId == _mouseReleasePanelId) {
        // zoom in one panel, reflect the range change in 
        // other panels
        for (int ii = 0; ii < (int) _waterfalls.size(); ii++) {
          WaterfallPlot *waterfall = _waterfalls[ii];
          if (ii == _mouseReleasePanelId) {
            // perform 2D zoom
            waterfall->setZoomLimits(_mousePressX, _mousePressY,
                                  _mouseReleaseX, _mouseReleaseY);
          } else {
            // perform zoom in range only
            waterfall->setZoomLimitsY(_mousePressY, _mouseReleaseY);
          }
        } // ii
      } else {
        // zoom box crosses panels, zoom in range only
        for (int ii = 0; ii < (int) _waterfalls.size(); ii++) {
          WaterfallPlot *waterfall = _waterfalls[ii];
          waterfall->setZoomLimitsY(_mousePressY, _mouseReleaseY);
        }
      }

    } else if (_mousePressPanelType == PANEL_IQPLOT &&
               _mouseReleasePanelType == PANEL_IQPLOT) {
      // iqplot zoom
      if (_mousePressPanelId == _mouseReleasePanelId) {
        // zoom in one panel, reflect the range change in 
        // other panels
        for (int ii = 0; ii < (int) _iqPlots.size(); ii++) {
          IqPlot *iqPlot = _iqPlots[ii];
          if (ii == _mouseReleasePanelId) {
            // perform 2D zoom
            iqPlot->setZoomLimits(_mousePressX, _mousePressY,
                                  _mouseReleaseX, _mouseReleaseY);
            // } else {
            //   // perform zoom in range only
            //   iqPlot->setZoomLimitsY(_mousePressY, _mouseReleaseY);
          }
        } // ii
      }

    } // if (_mousePressPanelType == PANEL_ASCOPE
    
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    _zoomWorld.setWorldLimits(_worldPressX, _worldPressY,
                              _worldReleaseX, _worldReleaseY);
    _setTransform(_zoomWorld.getTransform());

    // enable unzoom button

    _manager.enableUnzoomButton();
    
    // Update the window in the renderers
    
    _refresh();

  }
    
  // hide the rubber band

  _rubberBand->hide();

  update();
}


/*************************************************************************
 * paintEvent()
 */

void SpectraWidget::paintEvent(QPaintEvent *event)
{

  // check on time since rendered

  RadxTime now(RadxTime::NOW);
  double timeSinceLast = now - _timeLastRendered;
  if (timeSinceLast < _params.min_secs_between_rendering) {
    return;
  }
  _timeLastRendered = now;

  // clear plot

  QPainter painter(this);
  painter.save();
  painter.eraseRect(0, 0, width(), height());
  _zoomWorld.setClippingOn(painter);
  painter.restore();

  // render ascopes and iq plots

  if (_beam) {
    for (size_t ii = 0; ii < _ascopes.size(); ii++) {
      _ascopes[ii]->plotBeam(painter, _beam, _selectedRangeKm);
    }
    for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
      _waterfalls[ii]->plotBeam(painter, _beam, _nSamplesPlot, _selectedRangeKm);
    }
    for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
      _iqPlots[ii]->plotBeam(painter, _beam, _nSamplesPlot, _selectedRangeKm);
    }
  }

  // draw main title

  _drawMainTitle(painter);
  
  // draw averlays

  _drawOverlays(painter);

}


/*************************************************************************
 * resizeEvent()
 */

void SpectraWidget::resizeEvent(QResizeEvent * e)
{

  _ascopeHeight = height() - _titleMargin;
  _waterfallHeight = height() - _titleMargin;

  for (size_t ii = 0; ii < _ascopes.size(); ii++) {

    int xOffset = ii * _ascopeWidth;
    int yOffset = _titleMargin;
    _ascopes[ii]->setWindowGeom(_ascopeWidth, _ascopeHeight,
                                xOffset, yOffset);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "   ascopeWidth[" << ii << "]: "
           << _ascopes[ii]->getWidth() << endl;
      cerr << "  ascopeHeight[" << ii << "]: "
           << _ascopes[ii]->getHeight() << endl;
      cerr << "       xOffset[" << ii << "]: "
           << _ascopes[ii]->getXOffset() << endl;
      cerr << "       yOffset[" << ii << "]: "
           << _ascopes[ii]->getYOffset() << endl;
    }

  }
  
  for (size_t ii = 0; ii < _waterfalls.size(); ii++) {

    int xOffset = _waterfallStartIx + ii * _waterfallWidth;
    int yOffset = _titleMargin;
    _waterfalls[ii]->setWindowGeom(_waterfallWidth, _waterfallHeight,
                                xOffset, yOffset);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "   waterfallWidth[" << ii << "]: "
           << _waterfalls[ii]->getWidth() << endl;
      cerr << "  waterfallHeight[" << ii << "]: "
           << _waterfalls[ii]->getHeight() << endl;
      cerr << "       xOffset[" << ii << "]: "
           << _waterfalls[ii]->getXOffset() << endl;
      cerr << "       yOffset[" << ii << "]: "
           << _waterfalls[ii]->getYOffset() << endl;
    }

  }
  
  _iqGrossHeight = height() - _titleMargin;
  _iqGrossWidth = width() - _iqStartIx;
  _iqPlotWidth = _iqGrossWidth / _nIqCols;
  _iqPlotHeight = _iqGrossHeight / _nIqRows;

  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {

    int rowNum = ii / _nIqCols;
    int colNum = ii - rowNum * _nIqCols; 
    int xOffset = _iqStartIx + colNum * _iqPlotWidth;
    int yOffset = _titleMargin + rowNum * _iqPlotHeight; 

    _iqPlots[ii]->setWindowGeom(_iqPlotWidth, _iqPlotHeight,
                                xOffset, yOffset);
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "    iqPlotWidth[" << ii << "]: "
           << _iqPlots[ii]->getWidth() << endl;
      cerr << "   iqPlotHeight[" << ii << "]: "
           << _iqPlots[ii]->getHeight() << endl;
      cerr << "  iqPlotXOffset[" << ii << "]: "
           << _iqPlots[ii]->getXOffset() << endl;
      cerr << "  iqPlotYOffset[" << ii << "]: "
           << _iqPlots[ii]->getYOffset() << endl;
    }

  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "SpectraWidget::resizeEvent" << endl;
    cerr << "  width: " << width() << endl;
    cerr << "  height: " << height() << endl;
    cerr << "  _nIqRows: " << _nIqRows << endl;
    cerr << "  _nIqCols: " << _nIqCols << endl;
    cerr << "  _titleMargin: " << _titleMargin << endl;
    cerr << "  _iqGrossWidth: " << _iqGrossWidth << endl;
    cerr << "  _iqGrossHeight: " << _iqGrossHeight << endl;
    cerr << "  _iqPlotWidth: " << _iqPlotWidth << endl;
    cerr << "  _iqPlotHeight: " << _iqPlotHeight << endl;
  }
  
  _resetWorld(width(), height());

  _refresh();
  
}


/*************************************************************************
 * resize()
 */

void SpectraWidget::resize(int width, int height)
{

  setGeometry(0, 0, width, height);
  _resetWorld(width, height);

}

//////////////////////////////////////////////////////////////
// reset the pixel size of the world view

void SpectraWidget::_resetWorld(int width, int height)

{
  
  _fullWorld.resize(width / 3, height / 3);

  _fullWorld.setWindowOffsets(_iqStartIx, _titleMargin);
  
  _zoomWorld = _fullWorld;
  _setTransform(_fullWorld.getTransform());

}

/*************************************************************************
 * set mouse click point from external routine, to simulate and actual
 * mouse release event
 */

void SpectraWidget::setMouseClickPoint(double worldX,
                                       double worldY)
{

  if (_pointClicked) {

    _worldReleaseX = worldX;
    _worldReleaseY = worldY;
    
    _mouseReleaseX = _zoomWorld.getIxPixel(_worldReleaseX);
    _mouseReleaseY = _zoomWorld.getIyPixel(_worldReleaseY);
    
    // Update the window
  
    update();

  }

}

/*************************************************************************
 * increment/decrement the range in response to up/down arrow keys
 */

void SpectraWidget::changeRange(int nGatesDelta)
{
  setRange(_selectedRangeKm + nGatesDelta * _beam->getGateSpacingKm());
}

/*************************************************************************
 * set the range in km
 */

void SpectraWidget::setRange(double rangeKm)
{
  if (_beam != NULL) {
    _selectedRangeKm = rangeKm;
    double maxRange = _beam->getMaxRange();
    if (_params.set_max_range) {
      maxRange = _params.max_range_km;
    }
    if (_selectedRangeKm < _beam->getStartRangeKm()) {
      _selectedRangeKm = _beam->getStartRangeKm();
    } else if (_selectedRangeKm > maxRange) {
      _selectedRangeKm = maxRange;
    }
  }
  _computeSelectedGateNum();
  update();
}

/*************************************************************************
 * compute selected gate num from selected range
 */

void SpectraWidget::_computeSelectedGateNum()
{
  if (_beam != NULL) {
    _selectedGateNum = 
      (int) ((_selectedRangeKm - _beam->getStartRangeKm()) /
             _beam->getGateSpacingKm() + 0.5);
    if (_selectedGateNum < 0) {
      _selectedGateNum = 0;
    } else if (_selectedGateNum > _beam->getNGates() - 1) {
      _selectedGateNum = _beam->getNGates() - 1;
    }
    _selectedRangeKm = _beam->getStartRangeKm() +
      _selectedGateNum * _beam->getGateSpacingKm();
  }
}

/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * Draw the overlays, axes, legends etc
 */

void SpectraWidget::_drawOverlays(QPainter &painter)
{

  // draw panel dividing lines

  painter.save();
  QPen dividerPen(_params.main_window_panel_divider_color);
  dividerPen.setWidth(_params.main_window_panel_divider_line_width);
  painter.setPen(dividerPen);

  // borders

  {
    QLineF upperBorder(0, 0, width()-1, 0);
    painter.drawLine(upperBorder);
    QLineF lowerBorder(0, height()-1, width()-1, height()-1);
    painter.drawLine(lowerBorder);
    QLineF leftBorder(0, 0, 0, height()-1);
    painter.drawLine(leftBorder);
    QLineF rightBorder(width()-1, 0, width()-1, height()-1);
    painter.drawLine(rightBorder);
  }
    
  // line below title
  {
    QLineF topLine(0, _titleMargin, width(), _titleMargin);
    painter.drawLine(topLine);
  }

  // ascope right boundaries
  for (int ii = 0; ii < _nAscopes; ii++) {
    QLineF ascopeBoundary(_ascopeStartIx + _ascopeWidth * (ii+1),
                          _titleMargin,
                          _ascopeStartIx + _ascopeWidth * (ii+1),
                          height());
    painter.drawLine(ascopeBoundary);
  }

  // waterfall right boundaries
  for (int ii = 0; ii < _nWaterfalls; ii++) {
    QLineF waterfallBoundary(_waterfallStartIx + _waterfallWidth * (ii+1),
                             _titleMargin,
                             _waterfallStartIx + _waterfallWidth * (ii+1),
                             height());
    painter.drawLine(waterfallBoundary);
  }

  // iq panels lower boundaries

  for (int irow = 1; irow < _nIqRows; irow++) {
    QLineF lowerBoundary(_iqStartIx, _titleMargin + irow * _iqPlotHeight,
                         width(), _titleMargin + irow * _iqPlotHeight);
    painter.drawLine(lowerBoundary);
  }

  // iq panels right boundaries
  
  for (int icol = 1; icol < _nIqCols; icol++) {
    QLineF rightBoundary(_iqStartIx + icol * _iqPlotWidth, _titleMargin,
                         _iqStartIx + icol * _iqPlotWidth, height());
    painter.drawLine(rightBoundary);
  }
  painter.restore();
  
  // click point cross hairs
  
  if (_pointClicked) {
    
    painter.save();

    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

    painter.restore();

  }

#ifdef JUNK
  // store font
  
  QFont origFont = painter.font();
  
  // Set the painter to use the right color and font

  painter.setPen(_params.iqplot_axes_color);
  
  // axes and labels

  QFont font(origFont);
  font.setPointSizeF(_params.iqplot_axis_label_font_size);
  painter.setFont(font);

  // axes

  QColor lineColor(_params.iqplot_axes_color);
  QColor gridColor(_params.iqplot_grid_color);
  QColor textColor(_params.iqplot_labels_color);

  QFont labelFont(origFont);
  labelFont.setPointSizeF(_params.iqplot_axis_label_font_size);
  QFont valuesFont(origFont);
  valuesFont.setPointSizeF(_params.iqplot_tick_values_font_size);
  
  _zoomWorld.drawAxisBottom(painter, "xu", true, true, true, _xGridEnabled);
  _zoomWorld.drawAxisLeft(painter, "yu", true, true, true, _yGridEnabled);

  // y label

  painter.setPen(_params.iqplot_labels_color);
  _zoomWorld.drawYAxisLabelLeft(painter, "Amplitude (**)");
  
  // legends
  
  vector<string> legends;
  char text[1024];
  sprintf(text, "Legend1: %g", 1.0);
  legends.push_back(text);
  sprintf(text, "Legend2 lon: %g", 2.0);
  legends.push_back(text);

  if (_params.iqplot_plot_legend1) {
    switch (_params.iqplot_legend1_pos) {
      case Params::LEGEND_TOP_LEFT:
        _zoomWorld.drawLegendsTopLeft(painter, legends);
        break;
      case Params::LEGEND_TOP_RIGHT:
        _zoomWorld.drawLegendsTopRight(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_LEFT:
        _zoomWorld.drawLegendsBottomLeft(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_RIGHT:
        _zoomWorld.drawLegendsBottomRight(painter, legends);
        break;
      default: {}
    }
  }
    
  if (_params.iqplot_plot_legend2) {
    switch (_params.iqplot_legend2_pos) {
      case Params::LEGEND_TOP_LEFT:
        _zoomWorld.drawLegendsTopLeft(painter, legends);
        break;
      case Params::LEGEND_TOP_RIGHT:
        _zoomWorld.drawLegendsTopRight(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_LEFT:
        _zoomWorld.drawLegendsBottomLeft(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_RIGHT:
        _zoomWorld.drawLegendsBottomRight(painter, legends);
        break;
      default: {}
    }
  }
    
  // title
    
  font.setPointSizeF(_params.iqplot_title_font_size);
  painter.setFont(font);

  string radarName(_params.radar_name);
  string title;
  title = (radarName + "   SPECTRAL PLOTS   ");
  _zoomWorld.drawTitleTopCenter(painter, title);
  
  _zoomWorld.drawAxesBox(painter);

  // draw the color scale
  
  // const DisplayField &field = _manager.getSelectedField();
  // _zoomWorld.drawColorScale(field.getColorMap(), painter,
  //                           _params.iqplot_axis_label_font_size);
  
#endif
  
}

/*************************************************************************
 * _refresh()
 */

void SpectraWidget::_refresh()
{
  update();
}


////////////////////
// set the transform

void SpectraWidget::_setTransform(const QTransform &transform)
{
  
  _fullTransform = transform;
  _zoomTransform = transform;
  
}
  
/////////////////////////////////////////////////////////////	
// Title
    
void SpectraWidget::_drawMainTitle(QPainter &painter) 

{

  painter.save();

  // set the font and color
  
  QFont font = painter.font();
  font.setPointSizeF(_params.main_title_font_size);
  painter.setFont(font);
  painter.setPen(_params.main_title_color);

  string title("TIME SERIES PLOTS");

  if (_beam) {
    string rname(_beam->getInfo().get_radar_name());
    if (_params.override_radar_name) rname = _params.radar_name;
    title.append(":");
    title.append(rname);
    char dateStr[1024];
    DateTime beamTime(_beam->getTimeSecs());
    snprintf(dateStr, 1024, "%.4d/%.2d/%.2d",
             beamTime.getYear(), beamTime.getMonth(), beamTime.getDay());
    title.append(" ");
    title.append(dateStr);
    char timeStr[1024];
    int nanoSecs = _beam->getNanoSecs();
    snprintf(timeStr, 1024, "%.2d:%.2d:%.2d.%.3d",
             beamTime.getHour(), beamTime.getMin(), beamTime.getSec(),
             (nanoSecs / 1000000));
    title.append("-");
    title.append(timeStr);
    char rangeStr[1024];
    snprintf(rangeStr, 1024, "Selected range: %.3fkm  ", _selectedRangeKm);
    title.append(" ");
    title.append(rangeStr);
  }

  // get bounding rectangle
  
  QRect tRect(painter.fontMetrics().tightBoundingRect(title.c_str()));
  
  qreal xx = (qreal) ((width() / 2.0) - (tRect.width() / 2.0));
  qreal yy = (qreal) (_titleMargin - tRect.height()) / 2.0;
  QRectF bRect(xx, yy, tRect.width() + 6, tRect.height() + 6);
                      
  // draw the text
  
  painter.drawText(bRect, Qt::AlignTop, title.c_str());

  painter.restore();

}

/*************************************************************************
 * create ascope
 */

void SpectraWidget::_createAscope(int id)
  
{

  AscopePlot *ascope = new AscopePlot(this, _params, id);
  ascope->setMomentType(_params._ascope_moments[id]);

  WorldPlot &ascopeWorld = ascope->getFullWorld();
  
  ascopeWorld.setLeftMargin(_params.ascope_left_margin);
  ascopeWorld.setRightMargin(0);
  ascopeWorld.setTopMargin(0);
  ascopeWorld.setBottomMargin(_params.ascope_bottom_margin);
  ascopeWorld.setTitleTextMargin(_params.ascope_title_text_margin);
  ascopeWorld.setLegendTextMargin(_params.ascope_legend_text_margin);
  ascopeWorld.setAxisTextMargin(_params.ascope_axis_text_margin);

  ascopeWorld.setColorScaleWidth(0);
  
  ascopeWorld.setXAxisTickLen(_params.ascope_axis_tick_len);
  ascopeWorld.setXNTicksIdeal(_params.ascope_n_ticks_ideal);
  ascopeWorld.setYAxisTickLen(_params.ascope_axis_tick_len);
  ascopeWorld.setYNTicksIdeal(_params.ascope_n_ticks_ideal);

  ascopeWorld.setXAxisLabelsInside(_params.ascope_x_axis_labels_inside);
  ascopeWorld.setYAxisLabelsInside(_params.ascope_y_axis_labels_inside);

  ascopeWorld.setTitleFontSize(_params.ascope_title_font_size);
  ascopeWorld.setAxisLabelFontSize(_params.ascope_axis_label_font_size);
  ascopeWorld.setTickValuesFontSize(_params.ascope_tick_values_font_size);
  ascopeWorld.setLegendFontSize(_params.ascope_legend_font_size);

  ascopeWorld.setTitleColor(_params.ascope_title_color);
  ascopeWorld.setAxisLineColor(_params.ascope_axes_color);
  ascopeWorld.setAxisTextColor(_params.ascope_axes_color);
  ascopeWorld.setGridColor(_params.ascope_grid_color);

  int xOffset = id * _ascopeWidth;
  int yOffset = _titleMargin;
  ascopeWorld.setWindowGeom(_ascopeWidth, _ascopeHeight,
                            xOffset, yOffset);
  
  ascopeWorld.setWorldLimits(0.0, 0.0, 1.0, 1.0);

  _ascopes.push_back(ascope);
  
}

/*************************************************************************
 * configure ascope
 */

void SpectraWidget::_configureAscope(int id)
  
{

  int xOffset = id * _ascopeWidth;
  int yOffset = _titleMargin;
  _ascopes[id]->setWindowGeom(_ascopeWidth, _ascopeHeight,
                              xOffset, yOffset);
  
  if (_beam == NULL) {
    return;
  }

  Params::moment_type_t momentType = _ascopes[id]->getMomentType();
  double minVal = AscopePlot::getMinVal(momentType);
  double maxVal = AscopePlot::getMaxVal(momentType);
  
  double maxRange = _beam->getMaxRange();
  if (_params.set_max_range) {
    maxRange = _params.max_range_km;
  }
  
  _ascopes[id]->setWorldLimits(minVal, 0.0, maxVal, maxRange);
  
  update();

}

/*************************************************************************
 * create waterfall
 */

void SpectraWidget::_createWaterfall(int id)
  
{

  WaterfallPlot *waterfall = new WaterfallPlot(this, _params, id);
  waterfall->setPlotType(_params._waterfall_plots[id].plot_type);
  waterfall->setMedianFiltLen(_params._waterfall_plots[id].median_filter_len);
  waterfall->setFftWindow(_params._waterfall_plots[id].fft_window);
  waterfall->setUseAdaptFilt(_params._waterfall_plots[id].use_adaptive_filter);
  waterfall->setClutWidthMps(_params._waterfall_plots[id].clutter_model_width_mps);
  waterfall->setUseRegrFilt(_params._waterfall_plots[id].use_regression_filter);
  waterfall->setRegrOrder(_params._waterfall_plots[id].regression_order);
  
  WorldPlot &waterfallWorld = waterfall->getFullWorld();
  
  waterfallWorld.setLeftMargin(_params.waterfall_left_margin);
  waterfallWorld.setRightMargin(0);
  waterfallWorld.setTopMargin(0);
  waterfallWorld.setBottomMargin(_params.waterfall_bottom_margin);
  waterfallWorld.setTitleTextMargin(_params.waterfall_title_text_margin);
  waterfallWorld.setLegendTextMargin(_params.waterfall_legend_text_margin);
  waterfallWorld.setAxisTextMargin(_params.waterfall_axis_text_margin);

  waterfallWorld.setColorScaleWidth(0);
  
  waterfallWorld.setXAxisTickLen(_params.waterfall_axis_tick_len);
  waterfallWorld.setXNTicksIdeal(_params.waterfall_n_ticks_ideal);
  waterfallWorld.setYAxisTickLen(_params.waterfall_axis_tick_len);
  waterfallWorld.setYNTicksIdeal(_params.waterfall_n_ticks_ideal);

  waterfallWorld.setXAxisLabelsInside(_params.waterfall_x_axis_labels_inside);
  waterfallWorld.setYAxisLabelsInside(_params.waterfall_y_axis_labels_inside);

  waterfallWorld.setTitleFontSize(_params.waterfall_title_font_size);
  waterfallWorld.setAxisLabelFontSize(_params.waterfall_axis_label_font_size);
  waterfallWorld.setTickValuesFontSize(_params.waterfall_tick_values_font_size);
  waterfallWorld.setLegendFontSize(_params.waterfall_legend_font_size);

  waterfallWorld.setTitleColor(_params.waterfall_title_color);
  waterfallWorld.setAxisLineColor(_params.waterfall_axes_color);
  waterfallWorld.setAxisTextColor(_params.waterfall_axes_color);
  waterfallWorld.setGridColor(_params.waterfall_grid_color);

  int xOffset = id * _waterfallWidth;
  int yOffset = _titleMargin;
  waterfallWorld.setWindowGeom(_waterfallWidth, _waterfallHeight,
                               xOffset, yOffset);
  
  waterfallWorld.setWorldLimits(0.0, 0.0, 1.0, 1.0);

  _waterfalls.push_back(waterfall);
  
}

/*************************************************************************
 * configure waterfall
 */

void SpectraWidget::_configureWaterfall(int id)
  
{

  int xOffset = _waterfallStartIx + id * _waterfallWidth;
  int yOffset = _titleMargin;
  _waterfalls[id]->setWindowGeom(_waterfallWidth, _waterfallHeight,
                                 xOffset, yOffset);
  
  if (_beam == NULL) {
    return;
  }

  double maxRange = _beam->getMaxRange();
  if (_params.set_max_range) {
    maxRange = _params.max_range_km;
  }
  
  _waterfalls[id]->setWorldLimits(0.0, 0.0, _nSamplesPlot, maxRange);

  update();

}

/*************************************************************************
 * create IqPlot
 */

void SpectraWidget::_createIqPlot(int id)
  
{
  
  IqPlot *iqplot = new IqPlot(this, _params, id);
  iqplot->setPlotType(_params._iq_plots[id].plot_type);
  iqplot->setRxChannel(_params._iq_plots[id].rx_channel);
  iqplot->setFftWindow(_params._iq_plots[id].fft_window);
  iqplot->setMedianFiltLen(_params._iq_plots[id].median_filter_len);
  iqplot->setUseAdaptFilt(_params._iq_plots[id].use_adaptive_filter);
  iqplot->setPlotClutModel(_params._iq_plots[id].plot_clutter_model);
  iqplot->setClutModelWidthMps(_params._iq_plots[id].clutter_model_width_mps);
  iqplot->setUseRegrFilt(_params._iq_plots[id].use_regression_filter);
  iqplot->setRegrOrder(_params._iq_plots[id].regression_order);
  iqplot->setRegrFiltInterpAcrossNotch
    (_params._iq_plots[id].regression_filter_interp_across_notch);
  iqplot->setComputePlotRangeDynamically
    (_params._iq_plots[id].compute_plot_range_dynamically);

  WorldPlot &iqplotWorld = iqplot->getFullWorld();
  
  iqplotWorld.setLeftMargin(_params.iqplot_left_margin);
  iqplotWorld.setRightMargin(0);
  iqplotWorld.setTopMargin(0);
  iqplotWorld.setBottomMargin(_params.iqplot_bottom_margin);
  iqplotWorld.setTitleTextMargin(_params.iqplot_title_text_margin);
  iqplotWorld.setLegendTextMargin(_params.iqplot_legend_text_margin);
  iqplotWorld.setAxisTextMargin(_params.iqplot_axis_text_margin);

  iqplotWorld.setColorScaleWidth(0);
  
  iqplotWorld.setXAxisTickLen(_params.iqplot_axis_tick_len);
  iqplotWorld.setXNTicksIdeal(_params.iqplot_n_ticks_ideal);
  iqplotWorld.setYAxisTickLen(_params.iqplot_axis_tick_len);
  iqplotWorld.setYNTicksIdeal(_params.iqplot_n_ticks_ideal);

  iqplotWorld.setXAxisLabelsInside(_params.iqplot_x_axis_labels_inside);
  iqplotWorld.setYAxisLabelsInside(_params.iqplot_y_axis_labels_inside);

  iqplotWorld.setTitleFontSize(_params.iqplot_title_font_size);
  iqplotWorld.setAxisLabelFontSize(_params.iqplot_axis_label_font_size);
  iqplotWorld.setTickValuesFontSize(_params.iqplot_tick_values_font_size);
  iqplotWorld.setLegendFontSize(_params.iqplot_legend_font_size);

  iqplotWorld.setTitleColor(_params.iqplot_title_color);
  iqplotWorld.setAxisLineColor(_params.iqplot_axes_color);
  iqplotWorld.setAxisTextColor(_params.iqplot_axes_color);
  iqplotWorld.setGridColor(_params.iqplot_grid_color);

  int rowNum = id / _nIqCols;
  int colNum = id - rowNum * _nIqCols; 
  int xOffset = _iqStartIx + colNum * _iqPlotWidth;
  int yOffset = _titleMargin + rowNum * _iqPlotHeight; 

  iqplotWorld.setWindowGeom(_iqPlotWidth, _iqPlotHeight,
                            xOffset, yOffset);
  
  iqplotWorld.setWorldLimits(0.0, 0.0, 1.0, 1.0);

  _iqPlots.push_back(iqplot);
  
}

/*************************************************************************
 * configure the iqplot
 */

void SpectraWidget::_configureIqPlot(int id)
  
{

  int rowNum = id / _nIqCols;
  int colNum = id - rowNum * _nIqCols; 
  int xOffset = _iqStartIx + colNum * _iqPlotWidth;
  int yOffset = _titleMargin + rowNum * _iqPlotHeight; 
  
  _iqPlots[id]->setWindowGeom(_iqPlotWidth, _iqPlotHeight,
                              xOffset, yOffset);
  
  if (_beam == NULL) {
    return;
  }

  _iqPlots[id]->setWorldLimits(0.0, 0.0, _nSamplesPlot, 1.0);

  update();

}

/////////////////////////////////////////////////////////////	
// determine the selected panel

void SpectraWidget::_identSelectedPanel(int xx, int yy,
                                        panel_type_t &panelType,
                                        int &panelId)

{

  // first check for clicks in the top title bar

  if (yy < _titleMargin) {
    panelType = PANEL_TITLE;
    panelId = 0;
    return;
  }

  // then check for clicks in the ascope panels to the left

  if (xx >= _iqStartIx) {

    // IQ plot
    
    panelType = PANEL_IQPLOT;
    int icol = (xx - _iqStartIx) / _iqPlotWidth;
    int irow = (yy - _titleMargin) / _iqPlotHeight;
    panelId = irow * _nIqCols + icol;
    
  } else if (xx >= _waterfallStartIx) {

    // waterfall plot
    
    panelType = PANEL_WATERFALL;
    panelId = (xx - _waterfallStartIx) / _waterfallWidth;
    
  } else {

    // ascope plot
    
    panelType = PANEL_ASCOPE;
    panelId = (xx - _ascopeStartIx) / _ascopeWidth;
    
  }

}

//////////////////////////////////////////////////////////
// create and show context menu

void SpectraWidget::showContextMenu(const QPoint &pos) 
{

  _identSelectedPanel(pos.x(), pos.y(),
                      _contextMenuPanelType,
                      _contextMenuPanelId);

  if (_contextMenuPanelType == PANEL_ASCOPE) {
    _createAscopeContextMenu(pos);
  } else if (_contextMenuPanelType == PANEL_WATERFALL) {
    _createWaterfallContextMenu(pos);
  } else if (_contextMenuPanelType == PANEL_IQPLOT) {
    _createIqPlotContextMenu(pos);
  }

}

//////////////////////////////////////////////////////////
// context menu for Ascope

void SpectraWidget::_createAscopeContextMenu(const QPoint &pos) 
{

  QMenu contextMenu("AscopeMenu", this);

  // set the field selection menu

  int id = _contextMenuPanelId;
  QAction setToDbz("Set to DBZ", this);
  connect(&setToDbz, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::DBZ);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToDbz);
  
  QAction setToVel("Set to VEL", this);
  connect(&setToVel, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::VEL);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToVel);
  
  QAction setToWidth("Set to WIDTH", this);
  connect(&setToWidth, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::WIDTH);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToWidth);
  
  QAction setToNcp("Set to NCP", this);
  connect(&setToNcp, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::NCP);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToNcp);
  
  QAction setToSnr("Set to SNR", this);
  connect(&setToSnr, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::SNR);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToSnr);
  
  QAction setToDbm("Set to DBM", this);
  connect(&setToDbm, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::DBM);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToDbm);
  
  QAction setToZdr("Set to ZDR", this);
  connect(&setToZdr, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::ZDR);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToZdr);
  
  QAction setToLdr("Set to LDR", this);
  connect(&setToLdr, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::LDR);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToLdr);
  
  QAction setToRhohv("Set to RHOHV", this);
  connect(&setToRhohv, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::RHOHV);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToRhohv);
  
  QAction setToPhidp("Set to PHIDP", this);
  connect(&setToPhidp, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::PHIDP);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToPhidp);
  
  QAction setToKdp("Set to KDP", this);
  connect(&setToKdp, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setMomentType(Params::KDP);
            _configureAscope(id);
          } );
  contextMenu.addAction(&setToKdp);

  // unzoom action

  QAction unzoom("Unzoom", this);
  connect(&unzoom, &QAction::triggered,
          [this, id] () {
            for (size_t ii = 0; ii < _ascopes.size(); ii++) {
              _ascopes[ii]->unzoom();
            }
          } );
  if (_ascopes[id]->getIsZoomed()) {
    contextMenu.addAction(&unzoom);
  }

  // X grid lines on/off

  QAction xGridLinesOn("X grid lines on", this);
  connect(&xGridLinesOn, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setXGridLinesOn(true);
          } );
  QAction xGridLinesOff("X grid lines off", this);
  connect(&xGridLinesOff, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setXGridLinesOn(false);
          } );
  if (_ascopes[id]->getXGridLinesOn()) {
    contextMenu.addAction(&xGridLinesOff);
  } else {
    contextMenu.addAction(&xGridLinesOn);
  }
  
  // Y grid lines on/off

  QAction yGridLinesOn("Y grid lines on", this);
  connect(&yGridLinesOn, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setYGridLinesOn(true);
          } );
  QAction yGridLinesOff("Y grid lines off", this);
  connect(&yGridLinesOff, &QAction::triggered,
          [this, id] () {
            _ascopes[id]->setYGridLinesOn(false);
          } );
  if (_ascopes[id]->getYGridLinesOn()) {
    contextMenu.addAction(&yGridLinesOff);
  } else {
    contextMenu.addAction(&yGridLinesOn);
  }
  
  contextMenu.exec(this->mapToGlobal(pos));
  
}

//////////////////////////////////////////////////////////
// context menu for Waterfall

void SpectraWidget::_createWaterfallContextMenu(const QPoint &pos) 
{

  QMenu contextMenu("WaterfallMenu", this);

  ///////////////////////////////////////
  // select plot type sub-menu
  
  int id = _contextMenuPanelId;
  QMenu setPlotTypeMenu("Set Plot Type", &contextMenu);
  contextMenu.addMenu(&setPlotTypeMenu);
  
  QAction plotHc("Plot HC", &contextMenu);
  connect(&plotHc, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_HC);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotHc);
  
  QAction plotVc("Plot VC", &contextMenu);
  connect(&plotVc, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_VC);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotVc);
  
  QAction plotHx("Plot HX", &contextMenu);
  connect(&plotHx, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_HX);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotHx);
  
  QAction plotVx("Plot VX", &contextMenu);
  connect(&plotVx, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_VX);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotVx);
  
  QAction plotZdr("Plot ZDR", &contextMenu);
  connect(&plotZdr, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_ZDR);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotZdr);
  
  QAction plotPhidp("Plot PHIDP", &contextMenu);
  connect(&plotPhidp, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_PHIDP);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotPhidp);
  
  QAction plotSdevZdr("Plot SDEV_ZDR", &contextMenu);
  connect(&plotSdevZdr, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_SDEV_ZDR);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotSdevZdr);
  
  QAction plotSdevPhidp("Plot SDEV_PHIDP", &contextMenu);
  connect(&plotSdevPhidp, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_SDEV_PHIDP);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotSdevPhidp);
  
  QAction plotCmd("Plot CMD", &contextMenu);
  connect(&plotCmd, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setPlotType(Params::WATERFALL_CMD);
            _configureWaterfall(id);
          } );
  setPlotTypeMenu.addAction(&plotCmd);
  
  ///////////////////////////////////////
  // select FFT window menu
  
  QMenu setFftWindowMenu("Set FFT Window", &contextMenu);
  contextMenu.addMenu(&setFftWindowMenu);
  
  QAction setFftWindowRect("Rectangular", &contextMenu);
  connect(&setFftWindowRect, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_RECT);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowRect);

  QAction setFftWindowVonHann("VonHann", &contextMenu);
  connect(&setFftWindowVonHann, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_VONHANN);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowVonHann);

  QAction setFftWindowBlackman("Blackman", &contextMenu);
  connect(&setFftWindowBlackman, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_BLACKMAN);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowBlackman);

  QAction setFftWindowBlackmanNuttall("BlackmanNuttall", &contextMenu);
  connect(&setFftWindowBlackmanNuttall, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_BLACKMAN_NUTTALL);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowBlackmanNuttall);

  QAction setFftWindowTukey10("Tukey10", &contextMenu);
  connect(&setFftWindowTukey10, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_10);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey10);

  QAction setFftWindowTukey20("Tukey20", &contextMenu);
  connect(&setFftWindowTukey20, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_20);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey20);

  QAction setFftWindowTukey30("Tukey30", &contextMenu);
  connect(&setFftWindowTukey30, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_30);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey30);

  QAction setFftWindowTukey50("Tukey50", &contextMenu);
  connect(&setFftWindowTukey50, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_50);
            _configureWaterfall(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey50);

  ///////////////////////////////////////
  // filtering details menu
  
  QMenu setFilteringMenu("Set Filtering", &contextMenu);
  contextMenu.addMenu(&setFilteringMenu);
  
  QAction setMedianFiltLen("Set median filter len", &contextMenu);
  connect(&setMedianFiltLen, &QAction::triggered,
          [this, id] () {
            bool ok;
            int len = QInputDialog::getInt
              (this,
               tr("QInputDialog::getInt()"), tr("Set median filter len:"),
               _waterfalls[id]->getMedianFiltLen(),
               1, 100, 1, &ok);
            _waterfalls[id]->setMedianFiltLen(len);
            _configureWaterfall(id);
          } );
  setFilteringMenu.addAction(&setMedianFiltLen);
  
  QAction useAdaptFilter("Use adaptive filter", &contextMenu);
  useAdaptFilter.setCheckable(true);
  useAdaptFilter.setChecked
    (_waterfalls[id]->getUseAdaptFilt());
  connect(&useAdaptFilter, &QAction::triggered,
          [this, id] (bool state) {
            _waterfalls[id]->setUseAdaptFilt(state);
            _configureWaterfall(id);
          } );
  setFilteringMenu.addAction(&useAdaptFilter);

  QAction setClutterWidth("Set clutter model width", &contextMenu);
  connect(&setClutterWidth, &QAction::triggered,
          [this, id] () {
            bool ok;
            double width = QInputDialog::getDouble
              (this,
               tr("QInputDialog::getDouble()"), tr("Set clutter model width (mps):"),
               _waterfalls[id]->getClutWidthMps(), 0.05, 5.0, 2,
               &ok, Qt::WindowFlags());
            _waterfalls[id]->setClutWidthMps(width);
            _configureWaterfall(id);
          } );
  setFilteringMenu.addAction(&setClutterWidth);

  QAction useForsytheRegrFilter("Use regression filter", &contextMenu);
  useForsytheRegrFilter.setCheckable(true);
  useForsytheRegrFilter.setChecked
    (_waterfalls[id]->getUseRegrFilt());
  connect(&useForsytheRegrFilter, &QAction::triggered,
          [this, id] (bool state) {
            _waterfalls[id]->setUseRegrFilt(state);
            _configureWaterfall(id);
          } );
  setFilteringMenu.addAction(&useForsytheRegrFilter);

  QAction setRegressionOrder("Set regression order", &contextMenu);
  connect(&setRegressionOrder, &QAction::triggered,
          [this, id] () {
            bool ok;
            int order = QInputDialog::getInt
              (this,
               tr("QInputDialog::getInt()"), tr("Set regression order:"),
               _waterfalls[id]->getRegrOrder(),
               1, 100, 1, &ok);
            _waterfalls[id]->setRegrOrder(order);
            _configureWaterfall(id);
          } );
  setFilteringMenu.addAction(&setRegressionOrder);
  
  // unzoom action

  QAction unzoom("Unzoom", this);
  connect(&unzoom, &QAction::triggered,
          [this, id] () {
            for (size_t ii = 0; ii < _waterfalls.size(); ii++) {
              _waterfalls[ii]->unzoom();
            }
          } );
  if (_waterfalls[id]->getIsZoomed()) {
    contextMenu.addAction(&unzoom);
  }

  // X grid lines on/off

  QAction xGridLinesOn("X grid lines on", this);
  connect(&xGridLinesOn, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setXGridLinesOn(true);
          } );
  QAction xGridLinesOff("X grid lines off", this);
  connect(&xGridLinesOff, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setXGridLinesOn(false);
          } );
  if (_waterfalls[id]->getXGridLinesOn()) {
    contextMenu.addAction(&xGridLinesOff);
  } else {
    contextMenu.addAction(&xGridLinesOn);
  }
  
  // Y grid lines on/off

  QAction yGridLinesOn("Y grid lines on", this);
  connect(&yGridLinesOn, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setYGridLinesOn(true);
          } );
  QAction yGridLinesOff("Y grid lines off", this);
  connect(&yGridLinesOff, &QAction::triggered,
          [this, id] () {
            _waterfalls[id]->setYGridLinesOn(false);
          } );
  if (_waterfalls[id]->getYGridLinesOn()) {
    contextMenu.addAction(&yGridLinesOff);
  } else {
    contextMenu.addAction(&yGridLinesOn);
  }
  
  // show the context menu
  
  contextMenu.exec(this->mapToGlobal(pos));
  
}

//////////////////////////////////////////////////////////
// context menu for IqPlot

void SpectraWidget::_createIqPlotContextMenu(const QPoint &pos) 
{

  ///////////////////////////////////////
  // context menu
  
  QMenu contextMenu("IqPlotMenu", this);

  ///////////////////////////////////////
  // select plot type sub-menu
  
  int id = _contextMenuPanelId;
  QMenu setPlotTypeMenu("Set Plot Type", &contextMenu);
  contextMenu.addMenu(&setPlotTypeMenu);
  
  QAction plotSpectrumPower("Plot spectral power", &contextMenu);
  connect(&plotSpectrumPower, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::SPECTRAL_POWER);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotSpectrumPower);
  
  QAction plotSpectrumPhase("Plot spectral phase", &contextMenu);
  connect(&plotSpectrumPhase, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::SPECTRAL_PHASE);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotSpectrumPhase);
  
  QAction plotSpectrumZdr("Plot spectral zdr", &contextMenu);
  connect(&plotSpectrumZdr, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::SPECTRAL_ZDR);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotSpectrumZdr);
  
  QAction plotSpectrumPhidp("Plot spectral phidp", &contextMenu);
  connect(&plotSpectrumPhidp, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::SPECTRAL_PHIDP);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotSpectrumPhidp);
  
  QAction plotTsPower("Plot TS Power", &contextMenu);
  connect(&plotTsPower, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::TS_POWER);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotTsPower);
  
  QAction plotTsPhase("Plot TS Phase", &contextMenu);
  connect(&plotTsPhase, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::TS_PHASE);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotTsPhase);
  
  QAction plotIVals("Plot I Vals", &contextMenu);
  connect(&plotIVals, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::I_VALS);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotIVals);
  
  QAction plotQVals("Plot Q Vals", &contextMenu);
  connect(&plotQVals, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::Q_VALS);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotQVals);
  
  QAction plotIvsQ("Plot I vs Q", &contextMenu);
  connect(&plotIvsQ, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::I_VS_Q);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotIvsQ);
  
  QAction plotPhasor("PlotPhasor", &contextMenu);
  connect(&plotPhasor, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setPlotType(Params::PHASOR);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&plotPhasor);

  QAction dynamicRange("Compute plot range dynamically", &contextMenu);
  dynamicRange.setCheckable(true);
  dynamicRange.setChecked
    (_iqPlots[id]->getComputePlotRangeDynamically());
  connect(&dynamicRange, &QAction::triggered,
          [this, id] (bool state) {
            _iqPlots[id]->setComputePlotRangeDynamically(state);
            _configureIqPlot(id);
          } );
  setPlotTypeMenu.addAction(&dynamicRange);
  
  ///////////////////////////////////////
  // set channel sub-menu
  
  QMenu setChannelMenu("Set Channel", &contextMenu);
  contextMenu.addMenu(&setChannelMenu);
  
  QAction setChannelHc("Set channel HC", &contextMenu);
  connect(&setChannelHc, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setRxChannel(Params::CHANNEL_HC);
            _configureIqPlot(id);
          } );
  setChannelMenu.addAction(&setChannelHc);

  QAction setChannelVc("Set channel VC", &contextMenu);
  connect(&setChannelVc, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setRxChannel(Params::CHANNEL_VC);
            _configureIqPlot(id);
          } );
  setChannelMenu.addAction(&setChannelVc);

  QAction setChannelHx("Set channel HX", &contextMenu);
  connect(&setChannelHx, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setRxChannel(Params::CHANNEL_HX);
            _configureIqPlot(id);
          } );
  setChannelMenu.addAction(&setChannelHx);

  QAction setChannelVx("Set channel VX", &contextMenu);
  connect(&setChannelVx, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setRxChannel(Params::CHANNEL_VX);
            _configureIqPlot(id);
          } );
  setChannelMenu.addAction(&setChannelVx);

  ///////////////////////////////////////
  // select FFT window menu
  
  QMenu setFftWindowMenu("Set FFT Window", &contextMenu);
  contextMenu.addMenu(&setFftWindowMenu);
  
  QAction setFftWindowRect("Rectangular", &contextMenu);
  connect(&setFftWindowRect, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_RECT);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowRect);

  QAction setFftWindowVonHann("VonHann", &contextMenu);
  connect(&setFftWindowVonHann, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_VONHANN);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowVonHann);

  QAction setFftWindowBlackman("Blackman", &contextMenu);
  connect(&setFftWindowBlackman, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_BLACKMAN);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowBlackman);

  QAction setFftWindowBlackmanNuttall("BlackmanNuttall", &contextMenu);
  connect(&setFftWindowBlackmanNuttall, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_BLACKMAN_NUTTALL);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowBlackmanNuttall);

  QAction setFftWindowTukey10("Tukey10", &contextMenu);
  connect(&setFftWindowTukey10, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_10);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey10);

  QAction setFftWindowTukey20("Tukey20", &contextMenu);
  connect(&setFftWindowTukey20, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_20);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey20);

  QAction setFftWindowTukey30("Tukey30", &contextMenu);
  connect(&setFftWindowTukey30, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_30);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey30);

  QAction setFftWindowTukey50("Tukey50", &contextMenu);
  connect(&setFftWindowTukey50, &QAction::triggered,
          [this, id] () {
            _iqPlots[id]->setFftWindow(Params::FFT_WINDOW_TUKEY_50);
            _configureIqPlot(id);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey50);

  ///////////////////////////////////////
  // filtering details menu
  
  QMenu setFilteringMenu("Set Filtering", &contextMenu);
  contextMenu.addMenu(&setFilteringMenu);
  
  QAction setMedianFiltLen("Set median filter len", &contextMenu);
  connect(&setMedianFiltLen, &QAction::triggered,
          [this, id] () {
            bool ok;
            int len = QInputDialog::getInt
              (this,
               tr("QInputDialog::getInt()"), tr("Set median filter len:"),
               _iqPlots[id]->getMedianFiltLen(),
               1, 100, 1, &ok);
            _iqPlots[id]->setMedianFiltLen(len);
            _configureWaterfall(id);
          } );
  setFilteringMenu.addAction(&setMedianFiltLen);
  
  QAction useAdaptFilt("Use adaptive filter", &contextMenu);
  useAdaptFilt.setCheckable(true);
  useAdaptFilt.setChecked
    (_iqPlots[id]->getUseAdaptFilt());
  connect(&useAdaptFilt, &QAction::triggered,
          [this, id] (bool state) {
            _iqPlots[id]->setUseAdaptFilt(state);
            _configureIqPlot(id);
          } );
  setFilteringMenu.addAction(&useAdaptFilt);

  QAction plotClutModel("Plot clutter model", &contextMenu);
  plotClutModel.setCheckable(true);
  plotClutModel.setChecked
    (_iqPlots[id]->getPlotClutModel());
  connect(&plotClutModel, &QAction::triggered,
          [this, id] (bool state) {
            _iqPlots[id]->setPlotClutModel(state);
            _configureIqPlot(id);
          } );
  setFilteringMenu.addAction(&plotClutModel);

  QAction setClutterWidth("Set clutter model width", &contextMenu);
  connect(&setClutterWidth, &QAction::triggered,
          [this, id] () {
            bool ok;
            double width = QInputDialog::getDouble
              (this,
               tr("QInputDialog::getDouble()"), tr("Set clutter model width (mps):"),
               _iqPlots[id]->getClutModelWidthMps(), 0.05, 5.0, 2,
               &ok, Qt::WindowFlags());
            _iqPlots[id]->setClutModelWidthMps(width);
            _configureIqPlot(id);
          } );
  setFilteringMenu.addAction(&setClutterWidth);

  QAction useForsytheRegrFilter("Use regression filter", &contextMenu);
  useForsytheRegrFilter.setCheckable(true);
  useForsytheRegrFilter.setChecked
    (_iqPlots[id]->getUseRegrFilt());
  connect(&useForsytheRegrFilter, &QAction::triggered,
          [this, id] (bool state) {
            _iqPlots[id]->setUseRegrFilt(state);
            _configureIqPlot(id);
          } );
  setFilteringMenu.addAction(&useForsytheRegrFilter);

  QAction setRegressionOrder("Set regression order", &contextMenu);
  connect(&setRegressionOrder, &QAction::triggered,
          [this, id] () {
            bool ok;
            int order = QInputDialog::getInt
              (this,
               tr("QInputDialog::getInt()"), tr("Set regression order:"),
               _iqPlots[id]->getRegrOrder(),
               1, 100, 1, &ok);
            _iqPlots[id]->setRegrOrder(order);
            _configureIqPlot(id);
          } );
  setFilteringMenu.addAction(&setRegressionOrder);
  
  QAction regrInterpNotch("Regr interp across notch", &contextMenu);
  regrInterpNotch.setCheckable(true);
  regrInterpNotch.setChecked
    (_iqPlots[id]->getRegrFiltInterpAcrossNotch());
  connect(&regrInterpNotch, &QAction::triggered,
          [this, id] (bool state) {
            _iqPlots[id]->setRegrFiltInterpAcrossNotch(state);
            _configureIqPlot(id);
          } );
  setFilteringMenu.addAction(&regrInterpNotch);
  
  QAction setRegressionClutWidthFactor("Set regression clutter width factor (ss)", &contextMenu);
  connect(&setRegressionClutWidthFactor, &QAction::triggered,
          [this, id] () {
            bool ok;
            double wf = QInputDialog::getDouble
              (this,
               tr("QInputDialog::getInt()"), tr("Set regression clutter width factor (ss):"),
               _iqPlots[id]->getRegrClutWidthFactor(),
               1, 100, 1, &ok);
            _iqPlots[id]->setRegrClutWidthFactor(wf);
            _configureIqPlot(id);
          } );
  setFilteringMenu.addAction(&setRegressionClutWidthFactor);
  
  // show the context menu
  
  contextMenu.exec(this->mapToGlobal(pos));
  
}


