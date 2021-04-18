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
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "SpectraWidget.hh"
#include "SpectraMgr.hh"
#include "Beam.hh"
#include "AscopePlot.hh"
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
  _colorScaleWidth = _params.main_color_scale_width;

  _titleMargin = _params.main_window_title_margin;

  _nAscopes = _params.ascope_n_panels_in_spectra_window;
  _ascopeWidth = _params.ascope_width_in_spectra_window; // constant
  _ascopeHeight = 100;
  _ascopeGrossWidth = _ascopeWidth * _nAscopes;

  _nIqRows = _params.iqplots_n_rows;
  _nIqCols = _params.iqplots_n_columns;
  _nIqPlots = _nIqRows * _nIqCols;
  
  _iqGrossHeight = height() - _titleMargin;
  _iqGrossWidth = width() - _ascopeGrossWidth;
  _iqPlotWidth = _iqGrossWidth / _nIqCols;
  _iqPlotHeight = _iqGrossHeight / _nIqRows;

  _ascopesConfigured = false;
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

  // _fullWorld.setXAxisLabelsInside(_params.ascope_x_axis_labels_inside);
  // _fullWorld.setYAxisLabelsInside(_params.ascope_y_axis_labels_inside);

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
  _refresh();

  for (size_t ii = 0; ii < _ascopes.size(); ii++) {
    _ascopes[ii]->unzoom();
  }

  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
    _iqPlots[ii]->unzoom();
  }

  update();

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
  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {
    _iqPlots[ii]->setYGridLinesOn(state);
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

  _identSelectedPanel(_mouseReleaseX, _mouseReleaseY,
                      _mouseReleasePanelType, _mouseReleasePanelId);

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
        _selectedRangeKm = _beam->getClosestRange(_selectedRangeKm, _selectedGateNum);
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
  
  _iqGrossHeight = height() - _titleMargin;
  _iqGrossWidth = width() - _ascopeGrossWidth;
  _iqPlotWidth = _iqGrossWidth / _nIqCols;
  _iqPlotHeight = _iqGrossHeight / _nIqRows;

  for (size_t ii = 0; ii < _iqPlots.size(); ii++) {

    int rowNum = ii / _nIqCols;
    int colNum = ii - rowNum * _nIqCols; 
    int xOffset = _ascopeGrossWidth + colNum * _iqPlotWidth;
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
  update();
  
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

  _fullWorld.setWindowOffsets(_ascopeGrossWidth, _titleMargin);
  
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
    QLineF ascopeBoundary(_ascopeWidth * (ii+1), _titleMargin,
                          _ascopeWidth * (ii+1), height());
    painter.drawLine(ascopeBoundary);
  }

  // iq panels lower boundaries

  for (int irow = 1; irow < _nIqRows; irow++) {
    QLineF lowerBoundary(_ascopeGrossWidth, _titleMargin + irow * _iqPlotHeight,
                         width(), _titleMargin + irow * _iqPlotHeight);
    painter.drawLine(lowerBoundary);
  }

  // iq panels right boundaries
  
  for (int icol = 1; icol < _nIqCols; icol++) {
    QLineF rightBoundary(_ascopeGrossWidth + icol * _iqPlotWidth, _titleMargin,
                         _ascopeGrossWidth + icol * _iqPlotWidth, height());
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
  // painter.setWindow(0, 0, width(), height());

  // axes

  QColor lineColor(_params.iqplot_axes_color);
  QColor gridColor(_params.iqplot_grid_color);
  QColor textColor(_params.iqplot_labels_color);

  QFont labelFont(origFont);
  labelFont.setPointSizeF(_params.iqplot_axis_label_font_size);
  QFont valuesFont(origFont);
  valuesFont.setPointSizeF(_params.iqplot_tick_values_font_size);
  
  // _zoomWorld.drawRangeAxes(painter,
  //                          "xxx", _yGridEnabled,
  //                          lineColor, gridColor, textColor,
  //                          labelFont, valuesFont, true);
  
  // _zoomWorld.drawTimeAxes(painter,
  //                         _plotStartTime, _plotEndTime,
  //                         _xGridEnabled,
  //                         lineColor, gridColor, textColor,
  //                         labelFont, valuesFont,
  //                         false);

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

  // for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {

  //   FieldRenderer *field = _fieldRenderers[ifield];

  //   // If needed, create new image for this field
    
  //   if (size() != field->getImage()->size()) {
  //     field->createImage(width(), height());
  //   }

  //   // clear image
    
  //   field->getImage()->fill(_backgroundBrush.color().rgb());
    
  //   // set up rendering details

  //   field->setTransform(_zoomTransform);
  //   field->setUseHeight(_rangeAxisMode == Params::RANGE_AXIS_ALTITUDE);
  //   field->setDrawInstHt(_instHtLineEnabled);
    
  //   // Add pointers to the beams to be rendered

  //   if (ifield == _selectedField || field->isBackgroundRendered()) {

  //     std::vector< SpectraBeam* >::iterator beam;
  //     for (beam = _beams.begin(); beam != _beams.end(); ++beam) {
  //       (*beam)->setBeingRendered(ifield, true);
  //       field->addBeam(*beam);
  //     }
      
  //   }
    
  // } // ifield
  
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
  
  _ascopes[id]->setWorldLimits(minVal, 0.0,
                               maxVal, _beam->getMaxRange());

}

/*************************************************************************
 * create IqPlot
 */

void SpectraWidget::_createIqPlot(int id)
  
{
  
  IqPlot *iqplot = new IqPlot(this, _params, id);
  iqplot->setPlotType(_params._iqplots[id].plot_type);
  iqplot->setRxChannel(_params._iqplots[id].rx_channel);
  iqplot->setFftWindow(_params._iqplots[id].fft_window);

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
  int xOffset = _ascopeGrossWidth + colNum * _iqPlotWidth;
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
  int xOffset = _ascopeGrossWidth + colNum * _iqPlotWidth;
  int yOffset = _titleMargin + rowNum * _iqPlotHeight; 
  
  _iqPlots[id]->setWindowGeom(_iqPlotWidth, _iqPlotHeight,
                              xOffset, yOffset);
  
  if (_beam == NULL) {
    return;
  }

  _iqPlots[id]->setWorldLimits(0.0, 0.0, _nSamplesPlot, 1.0);

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

  if (xx < _ascopeGrossWidth) {
    panelType = PANEL_ASCOPE;
    panelId = xx / _ascopeWidth;
    return;
  }

  // we must therefore be in the spectra panels

  panelType = PANEL_IQPLOT;
  int icol = (xx - _ascopeGrossWidth) / _iqPlotWidth;
  int irow = (yy - _titleMargin) / _iqPlotHeight;
  panelId = irow * _nIqCols + icol;

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

  QAction setToDbz("Set to DBZ", this);
  connect(&setToDbz, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::DBZ);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToDbz);
  
  QAction setToVel("Set to VEL", this);
  connect(&setToVel, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::VEL);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToVel);
  
  QAction setToWidth("Set to WIDTH", this);
  connect(&setToWidth, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::WIDTH);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToWidth);
  
  QAction setToNcp("Set to NCP", this);
  connect(&setToNcp, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::NCP);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToNcp);
  
  QAction setToSnr("Set to SNR", this);
  connect(&setToSnr, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::SNR);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToSnr);
  
  QAction setToDbm("Set to DBM", this);
  connect(&setToDbm, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::DBM);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToDbm);
  
  QAction setToZdr("Set to ZDR", this);
  connect(&setToZdr, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::ZDR);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToZdr);
  
  QAction setToLdr("Set to LDR", this);
  connect(&setToLdr, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::LDR);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToLdr);
  
  QAction setToRhohv("Set to RHOHV", this);
  connect(&setToRhohv, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::RHOHV);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToRhohv);
  
  QAction setToPhidp("Set to PHIDP", this);
  connect(&setToPhidp, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::PHIDP);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToPhidp);
  
  QAction setToKdp("Set to KDP", this);
  connect(&setToKdp, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setMomentType(Params::KDP);
            _configureAscope(_contextMenuPanelId);
          } );
  contextMenu.addAction(&setToKdp);

  // unzoom action

  QAction unzoom("Unzoom", this);
  connect(&unzoom, &QAction::triggered,
          [this] () {
            for (size_t ii = 0; ii < _ascopes.size(); ii++) {
              _ascopes[ii]->unzoom();
            }
          } );
  if (_ascopes[_contextMenuPanelId]->getIsZoomed()) {
    contextMenu.addAction(&unzoom);
  }

  // X grid lines on/off

  QAction xGridLinesOn("X grid lines on", this);
  connect(&xGridLinesOn, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setXGridLinesOn(true);
          } );
  QAction xGridLinesOff("X grid lines off", this);
  connect(&xGridLinesOff, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setXGridLinesOn(false);
          } );
  if (_ascopes[_contextMenuPanelId]->getXGridLinesOn()) {
    contextMenu.addAction(&xGridLinesOff);
  } else {
    contextMenu.addAction(&xGridLinesOn);
  }
  
  // Y grid lines on/off

  QAction yGridLinesOn("Y grid lines on", this);
  connect(&yGridLinesOn, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setYGridLinesOn(true);
          } );
  QAction yGridLinesOff("Y grid lines off", this);
  connect(&yGridLinesOff, &QAction::triggered,
          [this] () {
            _ascopes[_contextMenuPanelId]->setYGridLinesOn(false);
          } );
  if (_ascopes[_contextMenuPanelId]->getYGridLinesOn()) {
    contextMenu.addAction(&yGridLinesOff);
  } else {
    contextMenu.addAction(&yGridLinesOn);
  }
  
  contextMenu.exec(this->mapToGlobal(pos));
  
}

//////////////////////////////////////////////////////////
// context menu for IqPlot

void SpectraWidget::_createIqPlotContextMenu(const QPoint &pos) 
{

  // context menu
  
  QMenu contextMenu("IqPlotMenu", this);

  // select plot type
  
  QMenu selectTypeMenu("Select Plot Type", &contextMenu);
  contextMenu.addMenu(&selectTypeMenu);

  QAction selectSpectrumPower("Select Spectrum Power", &contextMenu);
  connect(&selectSpectrumPower, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setPlotType(Params::SPECTRUM_POWER);
            _configureIqPlot(_contextMenuPanelId);
          } );
  selectTypeMenu.addAction(&selectSpectrumPower);
  
  QAction selectSpectrumPhase("Select Spectrum Phase", &contextMenu);
  connect(&selectSpectrumPhase, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setPlotType(Params::SPECTRUM_PHASE);
            _configureIqPlot(_contextMenuPanelId);
          } );
  selectTypeMenu.addAction(&selectSpectrumPhase);
  
  QAction selectTsPower("Select TS Power", &contextMenu);
  connect(&selectTsPower, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setPlotType(Params::TS_POWER);
            _configureIqPlot(_contextMenuPanelId);
          } );
  selectTypeMenu.addAction(&selectTsPower);
  
  QAction selectTsPhase("Select TS Phase", &contextMenu);
  connect(&selectTsPhase, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setPlotType(Params::TS_PHASE);
            _configureIqPlot(_contextMenuPanelId);
          } );
  selectTypeMenu.addAction(&selectTsPhase);
  
  QAction selectIandQ("Select I and Q", &contextMenu);
  connect(&selectIandQ, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setPlotType(Params::I_AND_Q);
            _configureIqPlot(_contextMenuPanelId);
          } );
  selectTypeMenu.addAction(&selectIandQ);
  
  QAction selectIvsQ("Select I vs Q", &contextMenu);
  connect(&selectIvsQ, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setPlotType(Params::I_VS_Q);
            _configureIqPlot(_contextMenuPanelId);
          } );
  selectTypeMenu.addAction(&selectIvsQ);
  
  QAction selectPhasor("SelectPhasor", &contextMenu);
  connect(&selectPhasor, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setPlotType(Params::PHASOR);
            _configureIqPlot(_contextMenuPanelId);
          } );
  selectTypeMenu.addAction(&selectPhasor);

  // set channel
  
  QMenu setChannelMenu("Set Channel", &contextMenu);
  contextMenu.addMenu(&setChannelMenu);
  
  QAction setChannelHc("Set channel HC", &contextMenu);
  connect(&setChannelHc, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setRxChannel(Params::CHANNEL_HC);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setChannelMenu.addAction(&setChannelHc);

  QAction setChannelVc("Set channel VC", &contextMenu);
  connect(&setChannelVc, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setRxChannel(Params::CHANNEL_VC);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setChannelMenu.addAction(&setChannelVc);

  QAction setChannelHx("Set channel HX", &contextMenu);
  connect(&setChannelHx, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setRxChannel(Params::CHANNEL_HX);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setChannelMenu.addAction(&setChannelHx);

  QAction setChannelVx("Set channel VX", &contextMenu);
  connect(&setChannelVx, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setRxChannel(Params::CHANNEL_VX);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setChannelMenu.addAction(&setChannelVx);

  // add FFT window menu
  
  _addFftWindowMenu(contextMenu);

  // show the context menu
  
  contextMenu.exec(this->mapToGlobal(pos));
  
}

//////////////////////////////////////////////////////////
// add the fftw window menu

void SpectraWidget::_addFftWindowMenu(QMenu &contextMenu) 
{

  // set FFT window
  
  QMenu setFftWindowMenu("Set FFT Window", &contextMenu);
  contextMenu.addMenu(&setFftWindowMenu);
  
  QAction setFftWindowRect("Rectangular", &contextMenu);
  connect(&setFftWindowRect, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_RECT);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowRect);

  QAction setFftWindowVonHann("VonHann", &contextMenu);
  connect(&setFftWindowVonHann, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_VONHANN);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowVonHann);

  QAction setFftWindowBlackman("Blackman", &contextMenu);
  connect(&setFftWindowBlackman, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_BLACKMAN);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowBlackman);

  QAction setFftWindowBlackmanNuttall("BlackmanNuttall", &contextMenu);
  connect(&setFftWindowBlackmanNuttall, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_BLACKMAN_NUTTALL);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowBlackmanNuttall);

  QAction setFftWindowTukey10("Tukey10", &contextMenu);
  connect(&setFftWindowTukey10, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_TUKEY_10);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey10);

  QAction setFftWindowTukey20("Tukey20", &contextMenu);
  connect(&setFftWindowTukey20, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_TUKEY_20);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey20);

  QAction setFftWindowTukey30("Tukey30", &contextMenu);
  connect(&setFftWindowTukey30, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_TUKEY_30);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey30);

  QAction setFftWindowTukey50("Tukey50", &contextMenu);
  connect(&setFftWindowTukey50, &QAction::triggered,
          [this] () {
            _iqPlots[_contextMenuPanelId]->setFftWindow(Params::FFT_WINDOW_TUKEY_50);
            _configureIqPlot(_contextMenuPanelId);
          } );
  setFftWindowMenu.addAction(&setFftWindowTukey50);

}


