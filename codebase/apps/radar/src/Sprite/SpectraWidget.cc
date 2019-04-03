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
#include <QMenu>

#include "SpectraWidget.hh"
#include "SpectraMgr.hh"
#include "Beam.hh"
#include "AscopePlot.hh"

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
        _currentBeam(NULL)

{

  _pointClicked = false;
  _colorScaleWidth = _params.main_color_scale_width;

  _nRows = _params.spectra_n_rows;
  _nCols = _params.spectra_n_columns;
  _titleMargin = _params.main_window_title_margin;

  _nAscopes = _params.ascope_n_panels_in_spectra_window;
  _ascopeWidth = _params.ascope_width_in_spectra_window; // constant
  _ascopeHeight = 100;
  _ascopeGrossWidth = _ascopeWidth * _nAscopes;
  _ascopesConfigured = false;

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
    // _configureAscope(ii);
  }

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

  _fullWorld.setLeftMargin(_params.spectra_left_margin);
  _fullWorld.setRightMargin(_params.spectra_right_margin);
  _fullWorld.setTopMargin(_params.spectra_top_margin);
  _fullWorld.setBottomMargin(_params.spectra_bottom_margin);
  _fullWorld.setTitleTextMargin(_params.spectra_title_text_margin);
  _fullWorld.setLegendTextMargin(_params.spectra_legend_text_margin);
  _fullWorld.setAxisTextMargin(_params.spectra_axis_text_margin);

  _fullWorld.setColorScaleWidth(0);

  _fullWorld.setXAxisTickLen(_params.spectra_axis_tick_len);
  _fullWorld.setXNTicksIdeal(_params.spectra_n_ticks_ideal);
  _fullWorld.setYAxisTickLen(_params.spectra_axis_tick_len);
  _fullWorld.setYNTicksIdeal(_params.spectra_n_ticks_ideal);

  // _fullWorld.setXAxisLabelsInside(_params.ascope_x_axis_labels_inside);
  // _fullWorld.setYAxisLabelsInside(_params.ascope_y_axis_labels_inside);

  _fullWorld.setTitleFontSize(_params.spectra_title_font_size);
  _fullWorld.setAxisLabelFontSize(_params.spectra_axis_label_font_size);
  _fullWorld.setTickValuesFontSize(_params.spectra_tick_values_font_size);
  _fullWorld.setLegendFontSize(_params.spectra_legend_font_size);

  _fullWorld.setTitleColor(_params.spectra_title_color);
  _fullWorld.setAxisLineColor(_params.spectra_axes_color);
  _fullWorld.setAxisTextColor(_params.spectra_axes_color);
  _fullWorld.setGridColor(_params.spectra_grid_color);

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
  update();
  // _updateRenderers();

}

/*************************************************************************
 * setGrids()
 */

void SpectraWidget::setXGridEnabled(bool state)
{
  _xGridEnabled = state;
  update();
}

void SpectraWidget::setYGridEnabled(bool state)
{
  _yGridEnabled = state;
  update();
}

/*************************************************************************
 * plot a beam
 */

void SpectraWidget::plotBeam(Beam *beam)

{

  if(_params.debug) {
    cerr << "======== SpectraWidget plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
  }

  _currentBeam = beam;

  if (_ascopes.size() > 0 && !_ascopesConfigured) {
    for (size_t ii = 0; ii < _ascopes.size(); ii++) {
      _configureAscope(ii);
    }
    _ascopesConfigured = true;
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

  QPixmap pixmap = QPixmap::grabWidget(this);
  QImage* image = new QImage(pixmap.toImage());
  return image;

}


/*************************************************************************
 * getPixmap()
 */

QPixmap* SpectraWidget::getPixmap()
{

  QPixmap* pixmap = new QPixmap(QPixmap::grabWidget(this));
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

  // if (e->button() == Qt::LeftButton) {
  //   cerr << "FFFFFFFFFFE - left button" << endl;
  // } else if (e->button() == Qt::MiddleButton) {
  //   cerr << "FFFFFFFFFFE - middle button" << endl;
  // } else if (e->button() == Qt::RightButton) {
  //   cerr << "FFFFFFFFFFE - right button" << endl;
  //   // right button is used for context menu
  //   return;
  // }

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
    
    // convert to real units of distance and time

    // double y_km = _worldReleaseY;
    // double x_secs = _worldReleaseX;
    // RadxTime clickTime(_plotStartTime.utime(), x_secs);
    
    // get closest ray to this time
    
    // double minDiff = 1.0e99;
    // const RadxRay *closestRay = NULL;
    // for (size_t ii = 0; ii < _beams.size(); ii++) {
    //   const RadxRay *ray = _beams[ii]->getRay();
    //   RadxTime rayTime(ray->getTimeSecs(), ray->getNanoSecs() / 1.0e9);
    //   double diff = fabs(rayTime - clickTime);
    //   if (diff < minDiff) {
    //     closestRay = ray;
    //     minDiff = diff;
    //   }
    // }

    // Emit a signal to indicate that the click location has changed

    _pointClicked = true;

    // if (closestRay != NULL) {
    //   emit locationClicked(x_secs, y_km, closestRay);
    // }

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

  RadxTime now(RadxTime::NOW);
  double timeSinceLast = now - _timeLastRendered;
  if (timeSinceLast < _params.min_secs_between_rendering) {
    return;
  }
  _timeLastRendered = now;

  QPainter painter(this);
  painter.save();
  painter.eraseRect(0, 0, width(), height());
  _zoomWorld.setClippingOn(painter);
  painter.restore();
  _drawOverlays(painter);
  _drawMainTitle(painter);
  
  // if we have a current beam, plot it
  if (_currentBeam) {
    for (size_t ii = 0; ii < _ascopes.size(); ii++) {
      _ascopes[ii]->plotBeam(painter, _currentBeam, _xGridEnabled, _yGridEnabled);
    }
  }

}


/*************************************************************************
 * resizeEvent()
 */

void SpectraWidget::resizeEvent(QResizeEvent * e)
{

  _iqGrossHeight = height() - _titleMargin;
  _iqGrossWidth = width() - _ascopeGrossWidth;
  _iqPanelWidths = _iqGrossWidth / _nCols;
  _iqPanelHeights = _iqGrossHeight / _nRows;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "SpectraWidget::resizeEvent" << endl;
    cerr << "  width: " << width() << endl;
    cerr << "  height: " << height() << endl;
    cerr << "  _nRows: " << _nRows << endl;
    cerr << "  _nCols: " << _nCols << endl;
    cerr << "  _titleMargin: " << _titleMargin << endl;
    cerr << "  _iqGrossWidth: " << _iqGrossWidth << endl;
    cerr << "  _iqGrossHeight: " << _iqGrossHeight << endl;
    cerr << "  _iqPanelWidths: " << _iqPanelWidths << endl;
    cerr << "  _iqPanelHeights: " << _iqPanelHeights << endl;
  }

  for (size_t ii = 0; ii < _ascopes.size(); ii++) {

    _ascopeHeight = height() - _titleMargin;
    int ascopeXOffset = ii * _ascopeWidth;
    int ascopeYOffset = _titleMargin;
    _ascopes[ii]->setWindowGeom(_ascopeWidth, _ascopeHeight,
                                ascopeXOffset, ascopeYOffset);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  ascopeWidth[" << ii << "]: "
           << _ascopes[ii]->getWidth() << endl;
      cerr << "  ascopeHeight[" << ii << "]: "
           << _ascopes[ii]->getHeight() << endl;
      cerr << "  ascopeXOffset[" << ii << "]: "
           << _ascopes[ii]->getXOffset() << endl;
      cerr << "  ascopeYOffset[" << ii << "]: "
           << _ascopes[ii]->getYOffset() << endl;
    }

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
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * Draw the overlays, axes, legends etc
 */

void SpectraWidget::_drawOverlays(QPainter &painter)
{

  // save painter state

  painter.save();

  // store font
  
  QFont origFont = painter.font();
  
  // Set the painter to use the right color and font

  painter.setPen(_params.spectra_axes_color);
  
  // axes and labels

  QFont font(origFont);
  font.setPointSizeF(_params.spectra_axis_label_font_size);
  painter.setFont(font);
  // painter.setWindow(0, 0, width(), height());

  // axes

  QColor lineColor(_params.spectra_axes_color);
  QColor gridColor(_params.spectra_grid_color);
  QColor textColor(_params.spectra_labels_color);

  QFont labelFont(origFont);
  labelFont.setPointSizeF(_params.spectra_axis_label_font_size);
  QFont valuesFont(origFont);
  valuesFont.setPointSizeF(_params.spectra_tick_values_font_size);
  
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

  painter.setPen(_params.spectra_labels_color);
  _zoomWorld.drawYAxisLabelLeft(painter, "Amplitude (**)");
  
  // legends
  
  vector<string> legends;
  char text[1024];
  sprintf(text, "Legend1: %g", 1.0);
  legends.push_back(text);
  sprintf(text, "Legend2 lon: %g", 2.0);
  legends.push_back(text);

  if (_params.spectra_plot_legend1) {
    switch (_params.spectra_legend1_pos) {
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
    
  if (_params.spectra_plot_legend2) {
    switch (_params.spectra_legend2_pos) {
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
    
  font.setPointSizeF(_params.spectra_title_font_size);
  painter.setFont(font);

  string radarName(_params.radar_name);
  string title;
  title = (radarName + "   SPECTRAL PLOTS   ");
  _zoomWorld.drawTitleTopCenter(painter, title);

  _zoomWorld.drawAxesBox(painter);

  // click point cross hairs
  
  if (_pointClicked) {
    
    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

  }

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

  // spectra panels lower boundaries

  for (int irow = 1; irow < _nRows; irow++) {
    QLineF lowerBoundary(_ascopeGrossWidth, _titleMargin + irow * _iqPanelHeights,
                         width(), _titleMargin + irow * _iqPanelHeights);
    painter.drawLine(lowerBoundary);
  }

  // spectra panels right boundaries

  for (int icol = 1; icol < _nCols; icol++) {
    QLineF rightBoundary(_ascopeGrossWidth + icol * _iqPanelWidths, _titleMargin,
                         _ascopeGrossWidth + icol * _iqPanelWidths, height());
    painter.drawLine(rightBoundary);
  }

  painter.restore();
  
  // reset painter state
  
  painter.restore();

  // draw the color scale

  // const DisplayField &field = _manager.getSelectedField();
  // _zoomWorld.drawColorScale(field.getColorMap(), painter,
  //                           _params.spectra_axis_label_font_size);
  
  return;
  
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
  
/*************************************************************************
 * update the renderers
 */

void SpectraWidget::_updateRenderers()

{
  
  // Update the window in the renderers
  
  // for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
  //   _fieldRenderers[field]->setTransform(_zoomTransform);
  // }

  // Refresh the images

  _refresh();

}

//////////////////////////////////
// initalize the plot start time

void SpectraWidget::setPlotStartTime(const RadxTime &plot_start_time,
                                     bool clearBeams /* = false */)
{
  
  _plotStartTime = plot_start_time;
  _plotEndTime = _plotStartTime + _timeSpanSecs;
  _pointClicked = false;

  // if (clearBeams) {
  //   for (size_t ii = 0; ii < _beams.size(); ii++) {
  //     Beam::deleteIfUnused(_beams[ii]);
  //   }
  //   _beams.clear();
  // }

  _refresh();

}

//////////////////////////////
// reset the plot start time

void SpectraWidget::resetPlotStartTime(const RadxTime &plot_start_time)
{

  // reset the plot start time

  setPlotStartTime(plot_start_time);

  // find the index for which the time is later than plot start time
  
  // vector<SpectraBeam *> toBeKept, toBeErased;
  // for (size_t ii = 0; ii < _beams.size(); ii++) {
  //   SpectraBeam *beam = _beams[ii];
  //   if ((beam->getBeamStartTime() - plot_start_time) >= 0.0) {
  //     toBeKept.push_back(beam);
  //   } else {
  //     toBeErased.push_back(beam);
  //   }
  // } // ii

  // erase beams
  
  // for (size_t ii = 0; ii < toBeErased.size(); ii++) {
  //   Beam::deleteIfUnused(toBeErased[ii]);
  // }
  // _beams.clear();
  // _beams = toBeKept;
  
  // set plot start time on remaining beams
  
  // for (size_t ii = 0; ii < _beams.size(); ii++) {
  //   _beams[ii]->resetPlotStartTime(_plotStartTime);
  // }

  // re-render

  _refresh();

}

/*************************************************************************
 * create the ascope
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
  ascopeWorld.setTitleTextMargin(_params.spectra_title_text_margin);
  ascopeWorld.setLegendTextMargin(_params.spectra_legend_text_margin);
  ascopeWorld.setAxisTextMargin(_params.spectra_axis_text_margin);

  ascopeWorld.setColorScaleWidth(0);
  
  ascopeWorld.setXAxisTickLen(_params.spectra_axis_tick_len);
  ascopeWorld.setXNTicksIdeal(_params.spectra_n_ticks_ideal);
  ascopeWorld.setYAxisTickLen(_params.spectra_axis_tick_len);
  ascopeWorld.setYNTicksIdeal(_params.spectra_n_ticks_ideal);

  ascopeWorld.setXAxisLabelsInside(_params.ascope_x_axis_labels_inside);
  ascopeWorld.setYAxisLabelsInside(_params.ascope_y_axis_labels_inside);

  ascopeWorld.setTitleFontSize(_params.spectra_title_font_size);
  ascopeWorld.setAxisLabelFontSize(_params.spectra_axis_label_font_size);
  ascopeWorld.setTickValuesFontSize(_params.spectra_tick_values_font_size);
  ascopeWorld.setLegendFontSize(_params.spectra_legend_font_size);

  ascopeWorld.setTitleColor(_params.ascope_title_color);
  ascopeWorld.setAxisLineColor(_params.spectra_axes_color);
  ascopeWorld.setAxisTextColor(_params.spectra_axes_color);
  ascopeWorld.setGridColor(_params.spectra_grid_color);

  int xOffset = id * _ascopeWidth;
  int yOffset = _titleMargin;
  ascopeWorld.setWindowGeom(_ascopeWidth, _ascopeHeight,
                            xOffset, yOffset);
  
  ascopeWorld.setWorldLimits(0.0, 0.0, 1.0, 1.0);

  _ascopes.push_back(ascope);
  
}

/*************************************************************************
 * configure the ascope
 */

void SpectraWidget::_configureAscope(int id)
  
{

  int xOffset = id * _ascopeWidth;
  int yOffset = _titleMargin;
  _ascopes[id]->setWindowGeom(_ascopeWidth, _ascopeHeight,
                              xOffset, yOffset);
  
  if (_currentBeam == NULL) {
    return;
  }

  Params::moment_type_t momentType = _ascopes[id]->getMomentType();
  double minVal = AscopePlot::getMinVal(momentType);
  double maxVal = AscopePlot::getMaxVal(momentType);
  
  _ascopes[id]->setWorldLimits(minVal, 0.0,
                               maxVal, _currentBeam->getMaxRange());

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

  string title("SPRITE");

  if (_currentBeam) {
    string rname(_currentBeam->getInfo().get_radar_name());
    if (_params.override_radar_name) rname = _params.radar_name;
    title.append(":");
    title.append(rname);
    char dateStr[1024];
    DateTime beamTime(_currentBeam->getTimeSecs());
    snprintf(dateStr, 1024, "%.4d/%.2d/%.2d",
             beamTime.getYear(), beamTime.getMonth(), beamTime.getDay());
    title.append(" ");
    title.append(dateStr);
    char timeStr[1024];
    int nanoSecs = _currentBeam->getNanoSecs();
    snprintf(timeStr, 1024, "%.2d:%.2d:%.2d.%.3d",
             beamTime.getHour(), beamTime.getMin(), beamTime.getSec(),
             (nanoSecs / 1000000));
    title.append("-");
    title.append(timeStr);
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

/////////////////////////////////////////////////////////////	
// determine the selected panel
    
void SpectraWidget::_identSelectedPanel(int xx, int yy,
                                        panel_type_t &panelType,
                                        int &panelId)

{

  // frist check for clicks in the top title bar

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
  int icol = (xx - _ascopeGrossWidth) / _iqPanelWidths;
  int irow = (yy - _titleMargin) / _iqPanelHeights;
  panelId = irow * _nRows + icol;

}

void SpectraWidget::showContextMenu(const QPoint &pos) 
{

  _identSelectedPanel(pos.x(), pos.y(),
                      _contextMenuPanelType,
                      _contextMenuPanelId);

  // if (_contextMenuPanelType == PANEL_ASCOPE) {
  //   cerr << "In ASCOPE, id: " << _contextMenuPanelId << endl;
  // } else if (_contextMenuPanelType == PANEL_SPECTRA) {
  //   cerr << "In SPECTRA, id: " << _contextMenuPanelId << endl;
  // } else {
  //   cerr << "In title" << endl;
  // }

  if (_contextMenuPanelType == PANEL_ASCOPE) {

    QMenu contextMenu("AscopeMenu", this);

    QAction setToDbz("Set to DBZ", this);
    connect(&setToDbz, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToDbz()));
    contextMenu.addAction(&setToDbz);
  
    QAction setToVel("Set to VEL", this);
    connect(&setToVel, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToVel()));
    contextMenu.addAction(&setToVel);
  
    QAction setToWidth("Set to WIDTH", this);
    connect(&setToWidth, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToWidth()));
    contextMenu.addAction(&setToWidth);
  
    QAction setToNcp("Set to NCP", this);
    connect(&setToNcp, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToNcp()));
    contextMenu.addAction(&setToNcp);
  
    QAction setToSnr("Set to SNR", this);
    connect(&setToSnr, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToSnr()));
    contextMenu.addAction(&setToSnr);
  
    QAction setToDbm("Set to DBM", this);
    connect(&setToDbm, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToDbm()));
    contextMenu.addAction(&setToDbm);
  
    QAction setToZdr("Set to ZDR", this);
    connect(&setToZdr, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToZdr()));
    contextMenu.addAction(&setToZdr);
  
    QAction setToLdr("Set to LDR", this);
    connect(&setToLdr, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToLdr()));
    contextMenu.addAction(&setToLdr);
  
    QAction setToRhohv("Set to RHOHV", this);
    connect(&setToRhohv, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToRhohv()));
    contextMenu.addAction(&setToRhohv);
  
    QAction setToPhidp("Set to PHIDP", this);
    connect(&setToPhidp, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToPhidp()));
    contextMenu.addAction(&setToPhidp);
  
    QAction setToKdp("Set to KDP", this);
    connect(&setToKdp, SIGNAL(triggered()), this,
            SLOT(ascopeSetFieldToKdp()));
    contextMenu.addAction(&setToKdp);
  
    QAction unzoom("Unzoom", this);
    connect(&unzoom, SIGNAL(triggered()), this,
            SLOT(ascopeUnzoom()));
    contextMenu.addAction(&unzoom);
  
    contextMenu.exec(this->mapToGlobal(pos));

  }

}

void SpectraWidget::ascopeSetFieldToDbz()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::DBZ);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToVel()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::VEL);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToWidth()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::WIDTH);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToSnr()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::SNR);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToNcp()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::NCP);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToDbm()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::DBM);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToZdr()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::ZDR);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToLdr()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::LDR);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToRhohv()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::RHOHV);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToPhidp()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::PHIDP);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeSetFieldToKdp()
{
  _ascopes[_contextMenuPanelId]->setMomentType(Params::KDP);
  _configureAscope(_contextMenuPanelId);
}

void SpectraWidget::ascopeUnzoom()
{
  for (size_t ii = 0; ii < _ascopes.size(); ii++) {
    _ascopes[ii]->unzoom();
  }
}
