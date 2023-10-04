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

#include "BscanWidget.hh"
#include "BscanManager.hh"

using namespace std;

BscanWidget::BscanWidget(QWidget* parent,
                         const BscanManager &manager,
                         const Params &params,
                         const vector<DisplayField *> &fields,
                         bool haveFilteredFields) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(params),
        _fields(fields),
        _haveFilteredFields(haveFilteredFields),
        _selectedField(0),
        _backgroundBrush(QColor(_params.background_color)),
        _scaledLabel(ScaledLabel::DistanceEng),
        _worldReleaseX(0),
        _worldReleaseY(0),
        _rubberBand(0)

{

  _pointClicked = false;
  
  _colorScaleWidth = _params.color_scale_width;

  _rangeGridEnabled = _params.bscan_draw_range_grid_lines;
  _timeGridEnabled = _params.bscan_draw_time_grid_lines;
  _instHtLineEnabled = _params.bscan_draw_instrument_height_line;
  _latlonLegendEnabled = _params.bscan_plot_starting_latlon_as_legend;
  _speedTrackLegendEnabled = _params.bscan_plot_mean_track_and_speed_as_legend;
  _distScaleEnabled = _params.bscan_add_distance_to_time_axis;

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

  // create the field renderers
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    FieldRenderer *fieldRenderer = 
      new FieldRenderer(_params, ii, *_fields[ii]);
    fieldRenderer->createImage(width(), height());
    _fieldRenderers.push_back(fieldRenderer);
  }
  
  // set up world views
  
  configureAxes(_params.bscan_range_axis_mode,
                _params.bscan_min_range_km,
                _params.bscan_max_range_km,
                _params.bscan_min_altitude_km,
                _params.bscan_max_altitude_km,
                _params.bscan_time_span_secs);
  
  _altitudeInFeet = _params.bscan_altitude_in_feet;
  _rangeInFeet = _params.bscan_range_in_feet;
  
}

/*************************************************************************
 * Destructor
 */

BscanWidget::~BscanWidget()
{

  // delete all of the dynamically created beams

  for (size_t i = 0; i < _beams.size(); ++i) {
    Beam::deleteIfUnused(_beams[i]);
  }
  _beams.clear();

  // Delete all of the field renderers

  for (size_t i = 0; i < _fieldRenderers.size(); ++i) {
    delete _fieldRenderers[i];
  }
  _fieldRenderers.clear();

}


/*************************************************************************
 * configure the axes
 */

void BscanWidget::configureAxes(Params::range_axis_mode_t range_axis_mode,
                                double min_range,
                                double max_range,
                                double min_altitude,
                                double max_altitude,
                                double time_span_secs)

{

  _rangeAxisMode = range_axis_mode;
  _minRange = min_range;
  _maxRangeKm = max_range;
  _minAltitude = min_altitude;
  _maxAltitude = max_altitude;
  _timeSpanSecs = time_span_secs;
  _plotEndTime = _plotStartTime + _timeSpanSecs;

  // set bottom margin - increase this if we are plotting the distance labels and ticks

  int bottomMargin = _params.bscan_bottom_margin;
  if (_distScaleEnabled) {
    bottomMargin += (int) (_params.bscan_axis_values_font_size * 3.0 / 2.0 + 0.5);
  }

  if (range_axis_mode == Params::RANGE_AXIS_UP) {
    _fullWorld.set(width(), height(),
                   _params.bscan_left_margin,
                   _params.bscan_right_margin,
                   _params.bscan_top_margin,
                   bottomMargin,
                   _colorScaleWidth,
                   0.0,
                   _minRange,
                   _timeSpanSecs,
                   _maxRangeKm,
                   _params.bscan_axis_tick_len,
                   _params.bscan_n_ticks_ideal,
                   _params.bscan_text_margin);
  } else if (range_axis_mode == Params::RANGE_AXIS_DOWN) {
    _fullWorld.set(width(), height(),
                   _params.bscan_left_margin,
                   _params.bscan_right_margin,
                   _params.bscan_top_margin,
                   bottomMargin,
                   _colorScaleWidth,
                   0.0,
                   _maxRangeKm,
                   _timeSpanSecs,
                   _minRange,
                   _params.bscan_axis_tick_len,
                   _params.bscan_n_ticks_ideal,
                   _params.bscan_text_margin);
  } else {
    _fullWorld.set(width(), height(),
                   _params.bscan_left_margin,
                   _params.bscan_right_margin,
                   _params.bscan_top_margin,
                   bottomMargin,
                   _colorScaleWidth,
                   0.0,
                   _minAltitude,
                   _timeSpanSecs,
                   _maxAltitude,
                   _params.bscan_axis_tick_len,
                   _params.bscan_n_ticks_ideal,
                   _params.bscan_text_margin);
  }
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());

  // refresh all paints

  _refreshImages();

}

/*************************************************************************
 * clear()
 */

void BscanWidget::clear()
{

  // Clear out the beam array
  
  for (size_t i = 0; i < _beams.size(); i++) {
    Beam::deleteIfUnused(_beams[i]);
  }
  _beams.clear();
  _pointClicked = false;
  
  // Now rerender the images
  
  _refreshImages();
  
}


/*************************************************************************
 * refresh()
 */

void BscanWidget::refresh()
{
  _refreshImages();
}

/*************************************************************************
 * unzoom the view
 */

void BscanWidget::unzoomView()
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

void BscanWidget::setRangeGridEnabled(bool state)
{
  _rangeGridEnabled = state;
  update();
}

void BscanWidget::setTimeGridEnabled(bool state)
{
  _timeGridEnabled = state;
  update();
}


void BscanWidget::setInstHtLineEnabled(bool state)
{
  _instHtLineEnabled = state;
  refresh();
}

void BscanWidget::setLatlonLegendEnabled(bool state)
{
  _latlonLegendEnabled = state;
  refresh();
}

void BscanWidget::setSpeedTrackLegendEnabled(bool state)
{
  _speedTrackLegendEnabled = state;
  refresh();
}

void BscanWidget::setDistScaleEnabled(bool state)
{
  _distScaleEnabled = state;
  refresh();
}

void BscanWidget::setAltitudeInFeet(bool state)
{
  _altitudeInFeet = state;
  _refreshImages();
}

void BscanWidget::setRangeInFeet(bool state)
{
  _rangeInFeet = state;
  _refreshImages();
}


/*************************************************************************
 * numBeams()
 */

size_t BscanWidget::getNumBeams() const
{
  return _beams.size();
}


/*************************************************************************
 * selectVar()
 */

void BscanWidget::selectVar(size_t index)
{

  // If the field index isn't actually changing, we don't need to do anything

  if (_selectedField == index) {
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> BscanWidget::selectVar() for field index: " 
         << index << endl;
  }

  // If this field isn't being rendered in the background, render all of
  // the beams for it

  if (!_fieldRenderers[index]->isBackgroundRendered()) {
    std::vector< BscanBeam* >::iterator beam;
    for (beam = _beams.begin(); beam != _beams.end(); ++beam) {
      (*beam)->setBeingRendered(index, true);
      _fieldRenderers[index]->addBeam(*beam);
    }
  }
  _performRendering();
  
  // Do any needed housekeeping when the field selection is changed

  _fieldRenderers[_selectedField]->unselectField();
  _fieldRenderers[index]->selectField();
  
  // Change the selected field index

  _selectedField = index;

  // Update the display

  update();

}

/*************************************************************************
 * clearVar()
 */

void BscanWidget::clearVar(size_t index)
{
  if (index >= _fieldRenderers.size())
    return;

  // Set the brush for every beam/gate for this field to use the background
  // color

  std::vector< BscanBeam* >::iterator beam;
  for (beam = _beams.begin(); beam != _beams.end(); ++beam)
    (*beam)->resetFieldBrush(index, &_backgroundBrush);
  
  if (index == _selectedField) {
    update();
  }

}

/*************************************************************************
 * turn on archive-style rendering - all fields
 */

void BscanWidget::activateArchiveRendering()
{
  for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
    _fieldRenderers[ii]->setBackgroundRenderingOn();
  }
}


/*************************************************************************
 * turn on reatlime-style rendering - non-selected fields in background
 */

void BscanWidget::activateRealtimeRendering()
{
  
  for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
    if (ii != _selectedField) {
      _fieldRenderers[ii]->activateBackgroundRendering();
    }
  }

}


/*************************************************************************
 * addBeam()
 */

void BscanWidget::addBeam(const RadxRay *ray,
                          double instHtKm,
                          const RadxTime &plot_start_time,
                          const RadxTime &beam_start_time,
                          const RadxTime &beam_end_time,
                          const std::vector< std::vector< double > > &beam_data,
                          const std::vector< DisplayField* > &fields)
{

  // check if time went backwards, and if so clear the beam vector

  if (_beams.size() > 0) {
    const Beam *prevBeam = _beams[_beams.size() - 1];
    const RadxRay *prevRay = prevBeam->getRay();
    RadxTime prevTime = prevRay->getRadxTime();
    RadxTime thisTime = ray->getRadxTime();
    if (thisTime < prevTime) {
      clear();
    }
  }
  
  // Create the new beam, to keep track of the display information.
  // Beam start and stop angles are adjusted here so that they always 
  // increase clockwise. Likewise, if a beam crosses the 0 degree boundary,
  // it is split into two beams, each of them again obeying the clockwise
  // rule. Prescribing these rules makes the beam culling logic a lot simpler.

  BscanBeam *beam = new BscanBeam(_params,
                                  ray,
                                  instHtKm,
                                  _fieldRenderers.size(),
                                  plot_start_time,
                                  _timeSpanSecs,
                                  beam_start_time, beam_end_time);
  beam->addClient();
  _beams.push_back(beam);

  beam->fillColors(beam_data, fields, &_backgroundBrush);

  // Add the new beams to the render lists for each of the fields
  
  for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
    if (field == _selectedField ||
        _fieldRenderers[field]->isBackgroundRendered()) {
      _fieldRenderers[field]->addBeam(beam);
    } else {
      beam->setBeingRendered(field, false);
    }
  }
  
  // run the threads to render the new beams
  
  _performRendering();

}


/*************************************************************************
 * backgroundColor()
 */

void BscanWidget::setBackgroundColor(const QColor &color)
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

QImage* BscanWidget::getImage()
{

  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;

}


/*************************************************************************
 * getPixmap()
 */

QPixmap* BscanWidget::getPixmap()
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

void BscanWidget::mousePressEvent(QMouseEvent *e)
{

  _rubberBand->setGeometry(QRect(e->pos(), QSize()));
  _rubberBand->show();

  _mousePressX = e->position().x();
  _mousePressY = e->position().y();

  _worldPressX = _zoomWorld.getXWorld(_mousePressX);
  _worldPressY = _zoomWorld.getYWorld(_mousePressY);

}


/*************************************************************************
 * mouseMoveEvent()
 */

void BscanWidget::mouseMoveEvent(QMouseEvent * e)
{
  // Zooming with the mouse

  int x = e->position().x();
  int y = e->position().y();
  int deltaX = x - _mousePressX;
  int deltaY = y - _mousePressY;

  QRect newRect = QRect(_mousePressX, _mousePressY, (int) deltaX, (int) deltaY);

  newRect = newRect.normalized();
  _rubberBand->setGeometry(newRect);

}


/*************************************************************************
 * mouseReleaseEvent()
 */

void BscanWidget::mouseReleaseEvent(QMouseEvent *e)
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

    double y_km = _worldReleaseY;
    double x_secs = _worldReleaseX;
    RadxTime clickTime(_plotStartTime.utime(), x_secs);
    
    // get closest ray to this time
    
    double minDiff = 1.0e99;
    const RadxRay *closestRay = NULL;
    for (size_t ii = 0; ii < _beams.size(); ii++) {
      const RadxRay *ray = _beams[ii]->getRay();
      RadxTime rayTime(ray->getTimeSecs(), ray->getNanoSecs() / 1.0e9);
      double diff = fabs(rayTime - clickTime);
      if (diff < minDiff) {
        closestRay = ray;
        minDiff = diff;
      }
    }

    // Emit a signal to indicate that the click location has changed

    _pointClicked = true;

    if (closestRay != NULL) {
      emit locationClicked(x_secs, y_km, closestRay);
    }

  } else {

    // mouse moved more than 20 pixels, so a zoom occurred
    
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    _zoomWorld.set(_worldPressX, _worldPressY, _worldReleaseX, _worldReleaseY);
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

void BscanWidget::paintEvent(QPaintEvent *event)
{

  RadxTime now(RadxTime::NOW);
  double timeSinceLast = now - _timeLastRendered;
  if (timeSinceLast < _params.bscan_min_secs_between_rendering_beams) {
    return;
  }
  _timeLastRendered = now;

  QPainter painter(this);
  painter.save();
  painter.eraseRect(0, 0, width(), height());
  _zoomWorld.setClippingOn(painter);
  painter.drawImage(0, 0, *(_fieldRenderers[_selectedField]->getImage()));
  painter.restore();
  _drawOverlays(painter);

}


/*************************************************************************
 * resizeEvent()
 */

void BscanWidget::resizeEvent(QResizeEvent * e)
{

  _resetWorld(width(), height());
  _refreshImages();
  update();

}


/*************************************************************************
 * resize()
 */

void BscanWidget::resize(int width, int height)
{

  setGeometry(0, 0, width, height);
  _resetWorld(width, height);

}

//////////////////////////////////////////////////////////////
// reset the pixel size of the world view

void BscanWidget::_resetWorld(int width, int height)

{

  _fullWorld.resize(width, height);
  _zoomWorld = _fullWorld;
  _setTransform(_fullWorld.getTransform());

}

/*************************************************************************
 * set mouse click point from external routine, to simulate and actual
 * mouse release event
 */

void BscanWidget::setMouseClickPoint(double worldX,
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

void BscanWidget::_drawOverlays(QPainter &painter)
{

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
  
  if (_rangeAxisMode == Params::RANGE_AXIS_ALTITUDE) {
    if (_altitudeInFeet) {
      _zoomWorld.drawRangeAxes(painter,
                               "kft", _rangeGridEnabled,
                               lineColor, gridColor, textColor,
                               labelFont, valuesFont, true);
    } else {
      _zoomWorld.drawRangeAxes(painter,
                               "km", _rangeGridEnabled,
                               lineColor, gridColor, textColor,
                               labelFont, valuesFont, false);
    }
  } else {
    if (_rangeInFeet) {
      _zoomWorld.drawRangeAxes(painter,
                               "kft", _rangeGridEnabled,
                               lineColor, gridColor, textColor,
                               labelFont, valuesFont, true);
    } else {
      _zoomWorld.drawRangeAxes(painter,
                               "km", _rangeGridEnabled,
                               lineColor, gridColor, textColor,
                               labelFont, valuesFont, false);
    }
  }
  
  _zoomWorld.drawTimeAxes(painter,
                          _plotStartTime, _plotEndTime,
                          _timeGridEnabled,
                          lineColor, gridColor, textColor,
                          labelFont, valuesFont,
                          _distScaleEnabled);
  
  // y label

  painter.setPen(_params.bscan_labels_color);
  if (_rangeAxisMode == Params::RANGE_AXIS_ALTITUDE) {
    if (_altitudeInFeet) {
      _zoomWorld.drawYAxisLabelLeft(painter, "Altitude MSL (kft)");
    } else {
      _zoomWorld.drawYAxisLabelLeft(painter, "Altitude MSL (km)");
    }
  } else {
    if (_rangeInFeet) {
      _zoomWorld.drawYAxisLabelLeft(painter, "Range (kft)");
    } else {
      _zoomWorld.drawYAxisLabelLeft(painter, "Range (km)");
    }
  }

  // compute info for legends and distance scale

  _computeSummaryDistanceSpeedTrack();

  // legends

  if (_latlonLegendEnabled) {
    if (_distLocs.size() > 1) {
      vector<string> legends;
      char text[1024];
      sprintf(text, "Start lat: %g", _startLat);
      legends.push_back(text);
      sprintf(text, "Start lon: %g", _startLon);
      legends.push_back(text);
      
      switch (_params.bscan_starting_latlon_legend_pos) {
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
  } // if (_latlonLegendEnabled ...
    
  if (_speedTrackLegendEnabled) {
    if (_distLocs.size() > 1) {
      vector<string> legends;
      char text[1024];
      sprintf(text, "Mean speed m/s: %.1f", _meanSpeedMps);
      legends.push_back(text);
      sprintf(text, "Mean dirn degT: %.1f", _meanDirnDeg);
      legends.push_back(text);
      
      switch (_params.bscan_mean_track_and_speed_legend_pos) {
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
  } // if (_speedTrackLegendEnabled ....

  if (_distScaleEnabled) {
    _plotDistanceOnTimeAxis(painter);
  }
    
  // title
    
  font.setPointSizeF(_params.bscan_title_font_size);
  painter.setFont(font);

  string radarName(_params.radar_name);
  string title;
  if (_params.use_field_label_in_title) {
    title = (radarName + "   BSCAN   -   " +
             _manager.getSelectedFieldLabel());
  } else {
    title = (radarName + "   BSCAN   -   " +
             _manager.getSelectedFieldName());
  }
  if (_manager.getSelectedFieldUnits().size() > 0) {
    title += (" (" + _manager.getSelectedFieldUnits() + ")");
  }
  _zoomWorld.drawTitleTopCenter(painter, title);

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

  const DisplayField &field = _manager.getSelectedField();
  _zoomWorld.drawColorScale(field.getColorMap(), painter,
                            _params.bscan_axis_label_font_size,
                            _params.text_color);
  
  return;
  
}

/*************************************************************************
 * _refreshImages()
 */

void BscanWidget::_refreshImages()
{

  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {

    FieldRenderer *field = _fieldRenderers[ifield];

    // If needed, create new image for this field
    
    if (size() != field->getImage()->size()) {
      field->createImage(width(), height());
    }

    // clear image
    
    field->getImage()->fill(_backgroundBrush.color().rgb());
    
    // set up rendering details

    field->setTransform(_zoomTransform);
    field->setUseHeight(_rangeAxisMode == Params::RANGE_AXIS_ALTITUDE);
    field->setDrawInstHt(_instHtLineEnabled);
    
    // Add pointers to the beams to be rendered

    if (ifield == _selectedField || field->isBackgroundRendered()) {

      std::vector< BscanBeam* >::iterator beam;
      for (beam = _beams.begin(); beam != _beams.end(); ++beam) {
	(*beam)->setBeingRendered(ifield, true);
	field->addBeam(*beam);
      }
      
    }
    
  } // ifield
  
  // call threads to do the rendering
  
  _performRendering();

  update();

}


////////////////////
// set the transform

void BscanWidget::_setTransform(const QTransform &transform)
{
  
  _fullTransform = transform;
  _zoomTransform = transform;
  
}
  
/*************************************************************************
 * update the renderers
 */

void BscanWidget::_updateRenderers()

{
  
  // Update the window in the renderers
  
  for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
    _fieldRenderers[field]->setTransform(_zoomTransform);
  }

  // Refresh the images

  _refreshImages();

}

//////////////////////////////////
// initalize the plot start time

void BscanWidget::setPlotStartTime(const RadxTime &plot_start_time,
                                   bool clearBeams /* = false */)
{
  
  _plotStartTime = plot_start_time;
  _plotEndTime = _plotStartTime + _timeSpanSecs;
  _pointClicked = false;

  if (clearBeams) {
    for (size_t ii = 0; ii < _beams.size(); ii++) {
      Beam::deleteIfUnused(_beams[ii]);
    }
    _beams.clear();
  }

  _refreshImages();

}

//////////////////////////////
// reset the plot start time

void BscanWidget::resetPlotStartTime(const RadxTime &plot_start_time)
{

  // reset the plot start time

  setPlotStartTime(plot_start_time);

  // find the index for which the time is later than plot start time
  
  vector<BscanBeam *> toBeKept, toBeErased;
  for (size_t ii = 0; ii < _beams.size(); ii++) {
    BscanBeam *beam = _beams[ii];
    if ((beam->getBeamStartTime() - plot_start_time) >= 0.0) {
      toBeKept.push_back(beam);
    } else {
      toBeErased.push_back(beam);
    }
  } // ii

  // erase beams
  
  for (size_t ii = 0; ii < toBeErased.size(); ii++) {
    Beam::deleteIfUnused(toBeErased[ii]);
  }
  _beams.clear();
  _beams = toBeKept;
  
  // set plot start time on remaining beams
  
  for (size_t ii = 0; ii < _beams.size(); ii++) {
    _beams[ii]->resetPlotStartTime(_plotStartTime);
  }

  // re-render

  _refreshImages();

}

/*************************************************************************
 * call the renderers for each field
 */

void BscanWidget::_performRendering()
{

  // start the rendering
  
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    if (ifield == _selectedField ||
	_fieldRenderers[ifield]->isBackgroundRendered()) {
      _fieldRenderers[ifield]->signalRunToStart();
    }
  } // ifield

  // wait for rendering to complete
  
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    if (ifield == _selectedField ||
	_fieldRenderers[ifield]->isBackgroundRendered()) {
      _fieldRenderers[ifield]->waitForRunToComplete();
    }
  } // ifield

  update();

}


/*************************************************************************
 * Compute the distance details, starting lat/lon and mean track/speed
 */

void BscanWidget::_computeSummaryDistanceSpeedTrack()
{

  // load up position data for computing distances

  _distLocs.clear();
  _startLat = 0.0;
  _startLon = 0.0;
  _meanSpeedMps = 0.0;
  _meanDirnDeg = 0.0;

  int nBeams = _beams.size();
  int nSegs = _params.bscan_n_segments_for_computing_distance;

  if (nBeams < nSegs + 1) {
    return;
  }

  int nBeamsPerSegment = nBeams / nSegs;
  if (nBeamsPerSegment < 1) {
    nBeamsPerSegment = 1;
  }
  
  double radarLat = _manager.getRadarLat();
  double radarLon = _manager.getRadarLon();
  
  int ibeam = 0;
  int iseg = 0;
  while (true) {
    BscanBeam *beam = _beams[ibeam];
    const RadxRay *ray = beam->getRay();
    if (ray->getGeoreference() != NULL) {
      radarLat = ray->getGeoreference()->getLatitude();
      radarLon = ray->getGeoreference()->getLongitude();
    }
    DistLoc loc(ibeam, ray->getRadxTime(), radarLat, radarLon);
    _distLocs.push_back(loc);
    if (ibeam == nBeams - 1) {
      break;
    }
    iseg++;
    ibeam += nBeamsPerSegment;
    if (ibeam > nBeams - 1) {
      ibeam = nBeams - 1;
    }
  }

  _startLat = _distLocs[0].lat;
  _startLon = _distLocs[0].lon;

  // compute the distances and directions

  _sumDistKm = 0.0;
  for (size_t ii = 1; ii < _distLocs.size(); ii++) {
    DistLoc &loc1 = _distLocs[ii-1];
    DistLoc &loc2 = _distLocs[ii];
    double distKm, trackDeg;
    PJGLatLon2RTheta(loc1.lat, loc1.lon, loc2.lat, loc2.lon, &distKm, &trackDeg);
    double deltaSecs = loc2.time - loc1.time;
    double speedMps = (distKm * 1000.0) / deltaSecs;
    _sumDistKm += distKm;
    loc2.speedMps = speedMps;
    loc2.dirnDeg = trackDeg;
    loc2.distKm = distKm;
    loc2.sumDistKm = _sumDistKm;
  }
  
  const DistLoc &locStart = _distLocs[0];
  const DistLoc &locEnd = _distLocs[_distLocs.size()-1];
  double totDistKm, meanTrackDeg;
  PJGLatLon2RTheta(locStart.lat, locStart.lon, 
                   locEnd.lat, locEnd.lon,
                   &totDistKm, &meanTrackDeg);

  double totSecs = locEnd.time - locStart.time;
  _meanSpeedMps = (_sumDistKm * 1000.0) / totSecs;
  _meanDirnDeg = meanTrackDeg;
  if (_meanDirnDeg < 0) {
    _meanDirnDeg += 360.0;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    for (size_t ii = 0; ii < _distLocs.size(); ii++) {
      DistLoc &loc = _distLocs[ii];
      cerr << "==========>> loc ii: " << ii << endl;
      cerr << "  beamNum: " << loc.beamNum << endl;
      cerr << "  time: " << loc.time.asString() << endl;
      cerr << "  lat: " << loc.lat << endl;
      cerr << "  lon: " << loc.lon << endl;
      cerr << "  speedMps: " << loc.speedMps << endl;
      cerr << "  dirnDeg: " << loc.dirnDeg << endl;
      cerr << "  distKm: " << loc.distKm << endl;
      cerr << "  sumDistKm: " << loc.sumDistKm << endl;
      cerr << "==============================" << endl;
    }
  }
    
}

/*************************************************************************
 * Add distance scale to the time axis
 */

void BscanWidget::_plotDistanceOnTimeAxis(QPainter &painter)

{

  // compute distance ticks

  int nTicksIdeal = _params.bscan_n_ticks_ideal;
  
  vector<double> tickDists =
    WorldPlot::linearTicks(0.0, _sumDistKm, nTicksIdeal);

  vector<RadxTime> tickTimes;
  for (size_t ii = 0; ii < tickDists.size(); ii++) {
    RadxTime tickTime = _getTimeForDistanceVal(tickDists[ii]);
    tickTimes.push_back(tickTime);
  }

  // plot them

  QColor lineColor(_params.bscan_axes_color);
  QColor textColor(_params.bscan_labels_color);

  QFont origFont = painter.font();
  QFont valuesFont(origFont);
  valuesFont.setPointSizeF(_params.bscan_axis_values_font_size);

  _zoomWorld.drawDistanceTicks(painter,
                               _plotStartTime,
                               tickDists, tickTimes,
                               lineColor, textColor, valuesFont);
  
}

/*************************************************************************
 * get time for a specified distance value
 */

RadxTime BscanWidget::_getTimeForDistanceVal(double distKm)

{

  if (_distLocs.size() == 0) {
    return RadxTime(0);
  }

  if (_distLocs.size() == 1) {
    return _distLocs[_distLocs.size()-1].time;
  }

  for (size_t ii = 0; ii < _distLocs.size() - 1; ii++) {
    DistLoc &locThis = _distLocs[ii];
    DistLoc &locNext = _distLocs[ii+1];
    if (distKm == locThis.sumDistKm) {
      return locThis.time;
    }
    if (distKm == locNext.sumDistKm) {
      return locNext.time;
    }
    if (distKm > locThis.sumDistKm && distKm < locNext.sumDistKm) {
      double fraction =
        (distKm - locThis.sumDistKm) / (locNext.sumDistKm - locThis.sumDistKm);
      double timeSpan = (locNext.time - locThis.time);
      double timeFrac = timeSpan * fraction;
      RadxTime timeForDist = locThis.time + timeFrac;
      // cerr << "====>>  ii, fraction timeSpan timeFrac time: "
      //      << ii << ", "
      //      << fraction << ", "
      //      << timeSpan << ", "
      //      << timeFrac << ", "
      //      << timeForDist.asString() << endl;
      return timeForDist;
    }
  }

  return _distLocs[_distLocs.size()-1].time;
  
}
