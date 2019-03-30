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

using namespace std;

SpectraWidget::SpectraWidget(QWidget* parent,
                             const SpectraMgr &manager,
                             const Params &params) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(params),
        _backgroundBrush(QColor(_params.background_color)),
        _scaledLabel(ScaledLabel::DistanceEng),
        _worldReleaseX(0),
        _worldReleaseY(0),
        _rubberBand(0)

{

  _pointClicked = false;
  
  _colorScaleWidth = _params.color_scale_width;

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
  
  configureAxes(_params.spectra_min_amplitude,
                _params.spectra_max_amplitude,
                _params.spectra_time_span_secs);
  
}

/*************************************************************************
 * Destructor
 */

SpectraWidget::~SpectraWidget()
{

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
  
  _fullWorld.setWindowGeom(width() / 3, height() / 3,
                           50, 50);

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
  _fullWorld.setAxisTickLabelsInside(_params.spectra_axis_tick_labels_inside);

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

  _refreshImages();

}

/*************************************************************************
 * clear()
 */

void SpectraWidget::clear()
{

  // Clear out the beam array
  
  _pointClicked = false;
  
  // Now rerender the images
  
  _refreshImages();
  
}


/*************************************************************************
 * refresh()
 */

void SpectraWidget::refresh()
{
  _refreshImages();
}

/*************************************************************************
 * unzoom the view
 */

void SpectraWidget::unzoomView()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _refreshImages();
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
    cerr << "======== Plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
  }

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
  
  _refreshImages();

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
    
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    _zoomWorld.setWorldLimits(_worldPressX, _worldPressY,
                              _worldReleaseX, _worldReleaseY);
    _setTransform(_zoomWorld.getTransform());

    // enable unzoom button

    _manager.enableZoomButton();
    
    // Update the window in the renderers
    
    _refreshImages();

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
  if (timeSinceLast < _params.spectra_min_secs_between_rendering) {
    return;
  }
  _timeLastRendered = now;

  QPainter painter(this);
  painter.save();
  painter.eraseRect(0, 0, width(), height());
  _zoomWorld.setClippingOn(painter);
  // painter.drawImage(0, 0, *(_fieldRenderers[_selectedField]->getImage()));
  painter.restore();
  _drawOverlays(painter);

}


/*************************************************************************
 * resizeEvent()
 */

void SpectraWidget::resizeEvent(QResizeEvent * e)
{

  _resetWorld(width(), height());
  _refreshImages();
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

  _fullWorld.setWindowOffsets(50, 50);

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
  title = (radarName + "   ASCOPE   ");
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

  // reset painter state
  
  painter.restore();

  // draw the color scale

  // const DisplayField &field = _manager.getSelectedField();
  // _zoomWorld.drawColorScale(field.getColorMap(), painter,
  //                           _params.spectra_axis_label_font_size);
  
  return;
  
}

/*************************************************************************
 * _refreshImages()
 */

void SpectraWidget::_refreshImages()
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
  
  // call threads to do the rendering
  
  _performRendering();

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

  _refreshImages();

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

  _refreshImages();

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

  _refreshImages();

}

/*************************************************************************
 * call the renderers for each field
 */

void SpectraWidget::_performRendering()
{

  // start the rendering
  
  // for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
  //   if (ifield == _selectedField ||
  //       _fieldRenderers[ifield]->isBackgroundRendered()) {
  //     _fieldRenderers[ifield]->signalRunToStart();
  //   }
  // } // ifield

  // wait for rendering to complete
  
  // for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
  //   if (ifield == _selectedField ||
  //       _fieldRenderers[ifield]->isBackgroundRendered()) {
  //     _fieldRenderers[ifield]->waitForRunToComplete();
  //   }
  // } // ifield

  update();

}


