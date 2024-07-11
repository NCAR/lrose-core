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
#include "HorizWidget.hh"
#include "CartManager.hh"
// #include "ParameterColorView.hh"
// #include "FieldColorController.hh"
// #include "DisplayFieldModel.hh"
// #include "BoundaryPointEditor.hh"

#include <toolsa/toolsa_macros.h>
#include <toolsa/LogStream.hh>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QErrorMessage>
#include <QRect>

#include "cidd.h"

using namespace std;



HorizWidget::HorizWidget(QWidget* parent,
                         const CartManager &manager) :
        CartWidget(parent, manager)
        
{

  _aspectRatio = _params.horiz_aspect_ratio;
  _colorScaleWidth = _params.color_scale_width;

  // initialoze world view

  // configureRange(_params.max_range_km);

  setGrids(_params.horiz_grids_on_at_startup);
  setRings(_params.horiz_range_rings_on_at_startup);
  setAngleLines(_params.horiz_azimuth_lines_on_at_startup);

  _isArchiveMode = false;
  _isStartOfSweep = true;

  _plotStartTime.set(0);
  _plotEndTime.set(0);
  _meanElev = -9999.0;
  _sumElev = 0.0;
  _nRays = 0.0;
  
  _openingFileInfoLabel = new QLabel("Opening file, please wait...", parent);
  _openingFileInfoLabel->setStyleSheet("QLabel { background-color : darkBlue; color : yellow; qproperty-alignment: AlignCenter; }");
  _openingFileInfoLabel->setVisible(false);

  //fires every 50ms. used for boundary editor to
  // (1) detect shift key down (changes cursor)
  // (2) get notified if user zooms in or out so the boundary can be rescaled
  // Todo: investigate implementing a listener pattern instead
  startTimer(50);
}

/*************************************************************************
 * Destructor
 */

HorizWidget::~HorizWidget()
{

  // delete all of the dynamically created beams
  
  // for (size_t i = 0; i < _ppiBeams.size(); ++i) {
  //   Beam::deleteIfUnused(_ppiBeams[i]);
  // }
  // _ppiBeams.clear();

}

/*************************************************************************
 * clear()
 */

void HorizWidget::clear()
{
  // Clear out the beam array
  
  // for (size_t i = 0; i < _ppiBeams.size(); i++) {
  //   Beam::deleteIfUnused(_ppiBeams[i]);
  // }
  // _ppiBeams.clear();
  
  // Now rerender the images
  
  _refreshImages();
  showOpeningFileMsg(false);
}

/*************************************************************************
 * selectVar()
 */

void HorizWidget::selectVar(const size_t index)
{

  // If the field index isn't actually changing, we don't need to do anything
  
  if (_selectedField == index) {
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> HorizWidget::selectVar() for field index: " 
         << index << endl;
  }

  // If this field isn't being rendered in the background, render all of
  // the beams for it

  // if (!_fieldRenderers[index]->isBackgroundRendered()) {
  //   std::vector< PpiBeam* >::iterator beam;
  //   for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
  //     (*beam)->setBeingRendered(index, true);
  //     _fieldRenderers[index]->addBeam(*beam);
  //   }
  // }
  _performRendering();

  // Do any needed housekeeping when the field selection is changed

  // _fieldRenderers[_selectedField]->unselectField();
  // _fieldRenderers[index]->selectField();
  
  // Change the selected field index

  _selectedField = index;

  // Update the display

  update();
}

/*************************************************************************
 * clearVar()
 */

void HorizWidget::clearVar(const size_t index)
{

  if ((int) index >= _params.fields_n) {
    return;
  }

  // Set the brush for every beam/gate for this field to use the background
  // color

  // std::vector< PpiBeam* >::iterator beam;
  // for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
  //   (*beam)->resetFieldBrush(index, &_backgroundBrush);
  // }
  
  if (index == _selectedField) {
    update();
  }

}


#ifdef JUNK
/*************************************************************************
 * addBeam()
 */

void HorizWidget::addBeam(const RadxRay *ray,
                          const float start_angle,
                          const float stop_angle,
                          const std::vector< std::vector< double > > &beam_data,
                          const std::vector< DisplayField* > &fields)
{

  LOG(DEBUG_VERBOSE) << "enter";

  // add a new beam to the display. 
  // The steps are:
  // 1. preallocate mode: find the beam to be drawn, or dynamic mode:
  //    create the beam(s) to be drawn.
  // 2. fill the colors for all variables in the beams to be drawn
  // 3. make the display list for the selected variables in the beams
  //    to be drawn.
  // 4. call the new display list(s)

  // std::vector< PpiBeam* > newBeams;

  // The start and stop angle MUST specify a clockwise fill for the sector.
  // Thus if start_angle > stop_angle, we know that we have crossed the 0
  // boundary, and must break it up into 2 beams.

  // Create the new beam(s), to keep track of the display information.
  // Beam start and stop angles are adjusted here so that they always 
  // increase clockwise. Likewise, if a beam crosses the 0 degree boundary,
  // it is split into two beams, each of them again obeying the clockwise
  // rule. Prescribing these rules makes the beam culling logic a lot simpler.

  // Normalize the start and stop angles.  I'm not convinced that this works
  // for negative angles, but leave it for now.

  double n_start_angle = start_angle - ((int)(start_angle/360.0))*360.0;
  double n_stop_angle = stop_angle - ((int)(stop_angle/360.0))*360.0;

  if (n_start_angle <= n_stop_angle) {

    // This beam does not cross the 0 degree angle.  Just add the beam to
    // the beam list.

    // PpiBeam* b = new PpiBeam(_params, ray, _fields.size(), 
    //                          n_start_angle, n_stop_angle);
    // b->addClient();
    // _cullBeams(b);
    // _ppiBeams.push_back(b);
    // newBeams.push_back(b);

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    // PpiBeam* b1 = new PpiBeam(_params, ray, _fields.size(), n_start_angle, 360.0);
    // b1->addClient();
    // _cullBeams(b1);
    // _ppiBeams.push_back(b1);
    // newBeams.push_back(b1);

    // Now add the portion of the beam to the right of the 0 degree point.

    // PpiBeam* b2 = new PpiBeam(_params, ray, _fields.size(), 0.0, n_stop_angle);
    // b2->addClient();
    // _cullBeams(b2);
    // _ppiBeams.push_back(b2);
    // newBeams.push_back(b2);

  }

  // compute angles and times in archive mode

  // if (newBeams.size() > 0) {
    
  //   if (_isArchiveMode) {

  //     if (_isStartOfSweep) {
  //       _plotStartTime = ray->getRadxTime();
  //       _meanElev = -9999.0;
  //       _sumElev = 0.0;
  //       _nRays = 0.0;
  //       _isStartOfSweep = false;
  //     }
  //     _plotEndTime = ray->getRadxTime();
  //     _sumElev += ray->getElevationDeg();
  //     _nRays++;
  //     _meanElev = _sumElev / _nRays;
  //     LOG(DEBUG_VERBOSE) << "isArchiveMode _nRays = " << _nRays;    
  //   } // if (_isArchiveMode) 
    
  // } // if (newBeams.size() > 0) 


  // if (_params.debug >= Params::DEBUG_VERBOSE &&
  //     _ppiBeams.size() % 10 == 0) {
  //   cerr << "==>> _ppiBeams.size(): " << _ppiBeams.size() << endl;
  // }
  // LOG(DEBUG_VERBOSE) << "number of new Beams " << newBeams.size();

  // newBeams has pointers to all of the newly added beams.  Render the
  // beam data.

  // for (size_t ii = 0; ii < newBeams.size(); ii++) {

  //   PpiBeam *beam = newBeams[ii];
    
  //   // Set up the brushes for all of the fields in this beam.  This can be
  //   // done independently of a Painter object.
    
  //   beam->fillColors(beam_data, fields, &_backgroundBrush);

  //   // Add the new beams to the render lists for each of the fields
    
  //   // for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
  //   //   if (field == _selectedField ||
  //   //       _fieldRenderers[field]->isBackgroundRendered()) {
  //   //     _fieldRenderers[field]->addBeam(beam);
  //   //   } else {
  //   //     beam->setBeingRendered(field, false);
  //   //   }
  //   // }
    
  // } /* endfor - beam */

  // Start the threads to render the new beams

  _performRendering();

  LOG(DEBUG_VERBOSE) << "exit";
}
#endif

/*************************************************************************
 * configureRange()
 */

void HorizWidget::configureRange(double max_range)
{

  // Save the specified values

  _maxRangeKm = max_range;

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setGridSpacing();
  
  // set world view

  int leftMargin = _params.horiz_left_margin;
  int rightMargin =  _params.horiz_right_margin;
  int topMargin =  _params.horiz_top_margin;
  int bottomMargin =  _params.horiz_bot_margin;
  int colorScaleWidth = _params.color_scale_width;
  int axisTickLen = _params.horiz_axis_tick_len;
  int nTicksIdeal = _params.horiz_n_ticks_ideal;
  int titleTextMargin = _params.horiz_title_text_margin;
  int legendTextMargin = _params.horiz_legend_text_margin;
  int axisTextMargin = _params.horiz_axis_text_margin;

  _fullWorld.setWindowGeom(width(), height(), 0, 0);
  
  _fullWorld.setWorldLimits(gd.h_win.cmin_x, gd.h_win.cmin_y,
                            gd.h_win.cmax_x, gd.h_win.cmax_y);
  
  _fullWorld.setLeftMargin(leftMargin);
  _fullWorld.setRightMargin(rightMargin);
  _fullWorld.setTopMargin(topMargin);
  _fullWorld.setBottomMargin(bottomMargin);
  _fullWorld.setTitleTextMargin(titleTextMargin);
  _fullWorld.setLegendTextMargin(legendTextMargin);
  _fullWorld.setAxisTextMargin(axisTextMargin);
  _fullWorld.setColorScaleWidth(colorScaleWidth);

  _fullWorld.setXAxisTickLen(_params.horiz_axis_tick_len);
  _fullWorld.setXNTicksIdeal(_params.horiz_n_ticks_ideal);
  _fullWorld.setYAxisTickLen(_params.horiz_axis_tick_len);
  _fullWorld.setYNTicksIdeal(_params.horiz_n_ticks_ideal);

  _fullWorld.setTitleFontSize(_params.horiz_title_font_size);
  _fullWorld.setAxisLabelFontSize(_params.horiz_axis_label_font_size);
  _fullWorld.setTickValuesFontSize(_params.horiz_tick_values_font_size);
  _fullWorld.setLegendFontSize(_params.horiz_legend_font_size);

  _fullWorld.setTitleColor(_params.horiz_title_color);
  _fullWorld.setAxisLineColor(_params.horiz_axes_color);
  _fullWorld.setAxisTextColor(_params.horiz_axes_color);
  _fullWorld.setGridColor(_params.horiz_grid_color);

  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();

  // Initialize the images used for double-buffering.  For some reason,
  // the window size is incorrect at this point, but that will be corrected
  // by the system with a call to resize().

  _refreshImages();
  
}

////////////////////////////////////////////////////////////////////////
// Used to notify BoundaryPointEditor if the user has zoomed in/out
// or is pressing the Shift key
// Todo: investigate implementing a listener pattern instead

void HorizWidget::timerEvent(QTimerEvent *event)
{
  bool doUpdate = false;
  // bool isBoundaryEditorVisible = _manager._boundaryEditorDialog->isVisible();
  // if (isBoundaryEditorVisible) {
  //   double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
    // user may have zoomed in or out, so update the polygon point boxes
    // so they are the right size on screen
    // doUpdate = BoundaryPointEditor::Instance()->updateScale(xRange);
  // }
  // bool isBoundaryFinished = BoundaryPointEditor::Instance()->isAClosedPolygon();

  // bool isShiftKeyDown =
  //   (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);

  // if ((isBoundaryEditorVisible && !isBoundaryFinished) ||
  //     (isBoundaryEditorVisible && isBoundaryFinished && isShiftKeyDown)){
  //   this->setCursor(Qt::CrossCursor);
  // } else {
    this->setCursor(Qt::ArrowCursor);
  // }
  
  if (doUpdate) {  //only update if something has changed
    update();
  }
}


/*************************************************************************
 * mouseReleaseEvent()
 */
void HorizWidget::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  QRect rgeom = _rubberBand->geometry();

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  _mouseReleaseX = pos.x();
  _mouseReleaseY = pos.y();

  // get click location in world coords

  if (rgeom.width() <= 20) {
    
    // Emit a signal to indicate that the click location has changed
    _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
    _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

#ifdef NOTNOW    
    // If boundary editor active, then interpret boundary mouse release event
    BoundaryPointEditor *editor = BoundaryPointEditor::Instance(); 
    if (_manager._boundaryEditorDialog->isVisible()) {
      if (editor->getCurrentTool() == BoundaryToolType::polygon) {
        if (!editor->isAClosedPolygon()) {
          editor->addPoint(_worldReleaseX, _worldReleaseY);
        } else { //polygon finished, user may want to insert/delete a point
          editor->checkToAddOrDelPoint(_worldReleaseX,
                                       _worldReleaseY);
    	}
      } else if (editor->getCurrentTool() == BoundaryToolType::circle) {
        if (editor->isAClosedPolygon()) {
          editor->checkToAddOrDelPoint(_worldReleaseX,
                                       _worldReleaseY);
        } else {
          editor->makeCircle(_worldReleaseX,
                             _worldReleaseY,
                             editor->getCircleRadius());
    	}
      }
    }
#endif

    double x_km = _worldReleaseX;
    double y_km = _worldReleaseY;
    _pointClicked = true;
    
    // get ray closest to click point
    
    const RadxRay *closestRay = _getClosestRay(x_km, y_km);
    
    // emit signal

    emit locationClicked(x_km, y_km, closestRay);
    
  } else {

    // mouse moved more than 20 pixels, so a zoom occurred
    
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    _worldReleaseX = _zoomWorld.getXWorld(_zoomCornerX);
    _worldReleaseY = _zoomWorld.getYWorld(_zoomCornerY);

    _zoomWorld.setWorldLimits(_worldPressX, _worldPressY, _worldReleaseX, _worldReleaseY);

    _setTransform(_zoomWorld.getTransform());

    _setGridSpacing();

    // enable unzoom button
    
    _manager.enableZoomButton();
    
    // Update the window in the renderers
    
    _refreshImages();

  }
    
  // hide the rubber band

  _rubberBand->hide();
  update();

}


////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *HorizWidget::_getClosestRay(double x_km, double y_km)

{

  double clickAz = atan2(y_km, x_km) * RAD_TO_DEG;
  double radarDisplayAz = 90.0 - clickAz;
  if (radarDisplayAz < 0.0) radarDisplayAz += 360.0;
  LOG(DEBUG) << "clickAz = " << clickAz << " from x_km, y_km = " 
                          << x_km << "," << y_km; 
  LOG(DEBUG) << "radarDisplayAz = " << radarDisplayAz << " from x_km, y_km = "
             << x_km << y_km;

  // double minDiff = 1.0e99;
  const RadxRay *closestRay = NULL;
  // for (size_t ii = 0; ii < _ppiBeams.size(); ii++) {
  //   const RadxRay *ray = _ppiBeams[ii]->getRay();
  //   double rayAz = ray->getAzimuthDeg();
  //   double diff = fabs(radarDisplayAz - rayAz);
  //   if (diff > 180.0) {
  //     diff = fabs(diff - 360.0);
  //   }
  //   if (diff < minDiff) {
  //     closestRay = ray;
  //     minDiff = diff;
  //   }
  // }

  if (closestRay != NULL)
    LOG(DEBUG) << "closestRay has azimuth " << closestRay->getAzimuthDeg();
  else
    LOG(DEBUG) << "Error: No ray found";
  return closestRay;

}

/*************************************************************************
 * _setGridSpacing()
 */

void HorizWidget::_setGridSpacing()
{

  double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
  double yRange = _zoomWorld.getYMaxWorld() - _zoomWorld.getYMinWorld();
  double diagonal = sqrt(xRange * xRange + yRange * yRange);

  if (diagonal <= 1.0) {
    _ringSpacing = 0.05;
  } else if (diagonal <= 2.0) {
    _ringSpacing = 0.1;
  } else if (diagonal <= 5.0) {
    _ringSpacing = 0.2;
  } else if (diagonal <= 10.0) {
    _ringSpacing = 0.5;
  } else if (diagonal <= 20.0) {
    _ringSpacing = 1.0;
  } else if (diagonal <= 50.0) {
    _ringSpacing = 2.0;
  } else if (diagonal <= 100.0) {
    _ringSpacing = 5.0;
  } else if (diagonal <= 200.0) {
    _ringSpacing = 10.0;
  } else if (diagonal <= 300.0) {
    _ringSpacing = 20.0;
  } else if (diagonal <= 400.0) {
    _ringSpacing = 25.0;
  } else if (diagonal <= 500.0) {
    _ringSpacing = 50.0;
  } else {
    _ringSpacing = 50.0;
  }

}


/*************************************************************************
 * _drawOverlays()
 */

void HorizWidget::_drawOverlays(QPainter &painter)
{

  // Don't try to draw rings if we haven't been configured yet or if the
  // rings or grids aren't enabled.
  
  if (!_ringsEnabled && !_gridsEnabled && !_angleLinesEnabled) {
    return;
  }

  // save painter state

  painter.save();

  // store font
  
  QFont origFont = painter.font();
  
  // Draw rings

  if (_ringSpacing > 0.0 && _ringsEnabled) {

    // Set up the painter
    
    painter.save();
    painter.setTransform(_zoomTransform);
    painter.setPen(_gridRingsColor);
  
    // set narrow line width
    QPen pen = painter.pen();
    pen.setWidth(0);
    painter.setPen(pen);

    double ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      QRectF rect(-ringRange, -ringRange, ringRange * 2.0, ringRange * 2.0);
      painter.drawEllipse(rect);
      ringRange += _ringSpacing;
    }
    painter.restore();

    // Draw the labels
    
    QFont font = painter.font();
    font.setPointSizeF(_params.range_ring_label_font_size);
    painter.setFont(font);
    // painter.setWindow(0, 0, width(), height());
    
    ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      double labelPos = ringRange * SIN_45;
      const string &labelStr = _scaledLabel.scale(ringRange);
      _zoomWorld.drawText(painter, labelStr, labelPos, labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, -labelPos, labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, labelPos, -labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, -labelPos, -labelPos, Qt::AlignCenter);
      ringRange += _ringSpacing;
    }

  } /* endif - draw rings */
  
  // Draw the grid

  if (_ringSpacing > 0.0 && _gridsEnabled)  {

    // Set up the painter
    
    painter.save();
    painter.setTransform(_zoomTransform);
    painter.setPen(_gridRingsColor);
  
    double ringRange = _ringSpacing;
    double maxRingRange = ringRange;
    while (ringRange <= _maxRangeKm) {

      _zoomWorld.drawLine(painter, ringRange, -_maxRangeKm, ringRange, _maxRangeKm);
      _zoomWorld.drawLine(painter, -ringRange, -_maxRangeKm, -ringRange, _maxRangeKm);
      _zoomWorld.drawLine(painter, -_maxRangeKm, ringRange, _maxRangeKm, ringRange);
      _zoomWorld.drawLine(painter, -_maxRangeKm, -ringRange, _maxRangeKm, -ringRange);
      
      maxRingRange = ringRange;
      ringRange += _ringSpacing;
    }
    painter.restore();

    _zoomWorld.specifyXTicks(-maxRingRange, _ringSpacing);
    _zoomWorld.specifyYTicks(-maxRingRange, _ringSpacing);

    _zoomWorld.drawAxisLeft(painter, "km", true, true, true, true);
    _zoomWorld.drawAxisRight(painter, "km", true, true, true, true);
    _zoomWorld.drawAxisTop(painter, "km", true, true, true, true);
    _zoomWorld.drawAxisBottom(painter, "km", true, true, true, true);
    
  }
  
  // Draw the azimuth lines

  if (_angleLinesEnabled) {

    // Set up the painter
    
    painter.save();
    painter.setPen(_gridRingsColor);
  
    // Draw the lines along the X and Y axes

    _zoomWorld.drawLine(painter, 0, -_maxRangeKm, 0, _maxRangeKm);
    _zoomWorld.drawLine(painter, -_maxRangeKm, 0, _maxRangeKm, 0);

    // Draw the lines along the 30 degree lines

    double end_pos1 = SIN_30 * _maxRangeKm;
    double end_pos2 = COS_30 * _maxRangeKm;
    
    _zoomWorld.drawLine(painter, end_pos1, end_pos2, -end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, end_pos1, -end_pos2, -end_pos1);
    _zoomWorld.drawLine(painter, -end_pos1, end_pos2, end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, -end_pos1, -end_pos2, end_pos1);

    painter.restore();

  }
  
  // click point cross hairs
  
  if (_pointClicked) {

    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;

    QPen pen(painter.pen());
    pen.setColor(_params.click_cross_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(_params.click_cross_line_width);
    painter.setPen(pen);

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

    /****** testing ******
    // do smart brush ...
  QImage qImage;
  qImage = *(_fieldRenderers[_selectedField]->getImage());
  // qImage.load("/h/eol/brenda/octopus.jpg");
  // get the Image from somewhere ...   
  // qImage.invertPixels();
  qImage.convertToFormat(QImage::Format_RGB32);

  // get the color of the selected pixel
  QRgb colorToMatch = qImage.pixel(_mouseReleaseX, _mouseReleaseY);
  // walk to all adjacent pixels of the same color and make them white

  vector<QPoint> pixelsToConsider;
  vector<QPoint> neighbors = {QPoint(-1, 1), QPoint(0, 1), QPoint(1, 1),
                              QPoint(-1, 0),               QPoint(1, 0),
                              QPoint(-1,-1), QPoint(0,-1), QPoint(1,-1)};

  pixelsToConsider.push_back(QPoint(_mouseReleaseX, _mouseReleaseY));
  while (!pixelsToConsider.empty()) {
    QPoint currentPix = pixelsToConsider.back();
    pixelsToConsider.pop_back();
    if (qImage.pixel(currentPix) ==  colorToMatch) {
      // set currentPix to white
      qImage.setPixelColor(currentPix, QColor("white"));
      // cout << "setting pixel " << currentPix.x() << ", " << currentPix.y() << " to white" << endl;
      // add the eight adjacent neighbors
      for (vector<QPoint>::iterator noffset = neighbors.begin(); 
           noffset != neighbors.end(); ++noffset) {
        QPoint neighbor;
        neighbor = currentPix + *noffset; // QPoint(-1,1);
        if (qImage.valid(neighbor)) {
          pixelsToConsider.push_back(neighbor);
        }
      } // end for neighbors iterator
    }
  }

  pixelsToConsider.clear();
  QPainter painter(this);
  painter.drawImage(0, 0, qImage);
    ****** end testing *****/

  }

  // reset painter state
  
  painter.restore();

  // draw the color scale

  int fieldNum = gd.h_win.page;
  const ColorMap &colorMap = *(gd.mrec[fieldNum]->colorMap);
  _zoomWorld.drawColorScale(colorMap, painter, _params.label_font_size);

  if (_archiveMode) {
    
    // add legends with time, field name and elevation angle

    vector<string> legends;
    char text[1024];

    // time legend

    snprintf(text, 1024, "Start time: %s", _plotStartTime.asString(0).c_str());
    legends.push_back(text);
    
    // radar and site name legend

    string radarName("unknown");
    // string radarName(_platform.getInstrumentName());
    // if (_params.override_radar_name) {
    //   radarName = _params.radar_name;
    // }
    string siteName("unknown");
    // string siteName(_platform.getInstrumentName());
    // if (_params.override_site_name) {
    //   siteName = _params.site_name;
    // }
    string radarSiteLabel = radarName;
    if (siteName.size() > 0 && siteName != radarName) {
      radarSiteLabel += "/";
      radarSiteLabel += siteName;
    }
    legends.push_back(radarSiteLabel);

    // field name legend

    // string fieldName = _fieldRenderers[_selectedField]->getField().getLabel();
    // snprintf(text, "Field: %s", fieldName.c_str());
    // legends.push_back(text);

    // elevation legend

    snprintf(text, 1024, "Elevation(deg): %.2f", _meanElev);
    legends.push_back(text);

    // nrays legend

    snprintf(text, 1024, "NRays: %g", _nRays);
    legends.push_back(text);
    
    painter.save();
    painter.setPen(QColor(_params.text_color)); // Qt::darkMagenta); // Qt::yellow);
    painter.setBrush(Qt::black);
    painter.setBackgroundMode(Qt::OpaqueMode);

    switch (_params.horiz_main_legend_pos) {
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

    // painter.setBrush(Qt::white);
    // painter.setBackgroundMode(Qt::TransparentMode);
    painter.restore();

  } // if (_archiveMode) {

}

void HorizWidget::showOpeningFileMsg(bool isVisible)
{
	_openingFileInfoLabel->setGeometry(width()/2 - 120, height()/2 -15, 200, 30);
	_openingFileInfoLabel->setVisible(isVisible);
  update();
}

///////////////////////////////////////////////////////////////////////////
// Draw text, with (X, Y) in screen space
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

void HorizWidget::_drawScreenText(QPainter &painter, const string &text,
                                int text_x, int text_y,
                                int flags)
  
{

  int ixx = text_x;
  int iyy = text_y;
	
  QRect tRect(painter.fontMetrics().tightBoundingRect(text.c_str()));
  QRect bRect(painter.fontMetrics().
              boundingRect(ixx, iyy,
                           tRect.width() + 2, tRect.height() + 2,
                           flags, text.c_str()));
    
  painter.drawText(bRect, flags, text.c_str());
    
}

/*************************************************************************
 * numBeams()
 */

// size_t HorizWidget::getNumBeams() const
// {
//   // return _ppiBeams.size();
//   return 0;
// }

/*************************************************************************
 * _beamIndex()
 */

int HorizWidget::_beamIndex(const double start_angle,
                          const double stop_angle)
{

  // Find where the center angle of the beam will fall within the beam array
  
  // int ii = (int)
  //   (_ppiBeams.size()*(start_angle + (stop_angle-start_angle)/2)/360.0);

  // // Take care of the cases at the ends of the beam list
  
  // if (ii < 0)
  //   ii = 0;
  // if (ii > (int)_ppiBeams.size() - 1)
  //   ii = _ppiBeams.size() - 1;

  // return ii;

  return 0;

}


/*************************************************************************
 * _cullBeams()
 */

#ifdef NOTNOW
void HorizWidget::_cullBeams(const PpiBeam *beamAB)
{
  // This routine examines the collection of beams, and removes those that are 
  // completely occluded by other beams. The algorithm gives precedence to the 
  // most recent beams; i.e. beams at the end of the _ppiBeams vector.
  //
  // Remember that there won't be any beams that cross angles through zero; 
  // otherwise the beam culling logic would be a real pain, and HorizWidget has
  // already split incoming beams into two, if it received a beam of this type.
  //
  // The logic is as follows. First of all, just consider the start and stop angle 
  // of a beam to be a linear region. We can diagram the angle interval of beam(AB) as:
  //         a---------b
  // 
  // The culling logic will compare all other beams (XY) to AB, looking for an overlap.
  // An example overlap might be:
  //         a---------b
  //    x---------y
  // 
  // If an overlap on beam XY is detected, the occluded region is recorded
  //   as the interval (CD):        
  //         a---------b
  //    x---------y
  //         c----d
  // 
  // The culling algorithm starts with the last beam in the list, and compares it with all
  // preceeding beams, setting their overlap regions appropriately.
  // Then the next to the last beam is compared with all preceeding beams.
  // Previously found occluded regions will be expanded as they are detected.
  // 
  // Once the occluded region spans the entire beam, then the beam is known 
  // to be hidden, and it doesn't need to be tested any more, nor is it it used as a 
  // test on other beams.
  //
  // After the list has been completly processed in this manner, the completely occluded 
  // beams are removed.
  // .
  // Note now that if the list is rendered from beginning to end, the more recent beams will
  // overwrite the portions of previous beams that they share.
  //

  // NOTE - This algorithm doesn't handle beams that are occluded in different
  // subsections.  For example, the following would be handled as a hidden
  // beam even though the middle of the beam is still visible:
  //         a---------b    c--------d
  //              x-------------y

  // Do nothing if we don't have any beams in the list

  if (_ppiBeams.size() < 1)
    return;

  // Look through all of the beams in the list and record any place where
  // this beam occludes any other beam.

  bool need_to_cull = false;
  
  // Save the angle information for easier processing.
  
  double a = beamAB->startAngle;
  double b = beamAB->stopAngle;

  // Look at all of the beams in the list to see if any are occluded by this
  // new beam

  for (size_t j = 0; j < _ppiBeams.size(); ++j)
  {
    // Pull the beam from the list for ease of coding

    PpiBeam *beamXY = _ppiBeams[j];

    // If this beam has alread been marked hidden, we don't need to 
    // look at it.

    if (beamXY->hidden)
      continue;
      
    // Again, save the angles for easier coding

    double x = beamXY->startAngle;
    double y = beamXY->stopAngle;

    if (b <= x || a >= y)
    {
      //  handles these cases:
      //  a-----b                a-----b
      //           x-----------y
      //  
      // they don't overlap at all so do nothing
    }
    else if (a <= x && b >= y)
    {
      //     a------------------b
      //        x-----------y
      // completely covered

      beamXY->hidden = true;
      need_to_cull = true;
    }
    else if (a <= x && b <= y)
    {
      //   a-----------b
      //        x-----------y
      //
      // We know that b > x because otherwise this would have been handled
      // in the first case above.

      // If the right part of this beam is already occluded, we can just
      // mark the beam as hidden at this point.  Otherwise, we update the
      // c and d values.

      if (beamXY->rightEnd == y)
      {
	beamXY->hidden = true;
	need_to_cull = true;
      }
      else
      {
	beamXY->leftEnd = x;
	if (beamXY->rightEnd < b)
	  beamXY->rightEnd = b;
      }
    }
    else if (a >= x && b >= y)
    {
      //       a-----------b
      //   x-----------y
      //
      // We know that a < y because otherwise this would have been handled
      // in the first case above.
      
      // If the left part of this beam is already occluded, we can just
      // mark the beam as hidden at this point.  Otherwise, we update the
      // c and d values.

      if (beamXY->leftEnd == x)
      {
	beamXY->hidden = true;
	need_to_cull = true;
      }
      else
      {
	beamXY->rightEnd = y;
	if (a < beamXY->leftEnd)
	  beamXY->leftEnd = a;
      }
    }
    else
    {
      // all that is left is this pathological case:
      //     a-------b
      //   x-----------y
      //
      // We need to extend c and d, if the are inside of a and b.  We know
      // that a != x and b != y because otherwise this would have been
      // handled in the third case above.

      if (beamXY->leftEnd > a)
	beamXY->leftEnd = a;
      if (beamXY->rightEnd < b)
	beamXY->rightEnd = b;
	      
    } /* endif */
  } /* endfor - j */

  // Now actually cull the list

  if (need_to_cull)
  {
    // Note that i has to be an int rather than a size_t since we are going
    // backwards through the list and will end when i < 0.

    for (int i = _ppiBeams.size()-1; i >= 0; i--)
    {
      // Delete beams who are hidden but aren't currently being rendered.
      // We can get the case where we have hidden beams that are being
      // rendered when we do something (like resizing) that causes us to 
      // have to rerender all of the current beams.  During the rerendering,
      // new beams continue to come in and will obscure some of the beams
      // that are still in the rendering queue.  These beams will be deleted
      // during a later pass through this loop.

      if (_ppiBeams[i]->hidden && !_ppiBeams[i]->isBeingRendered())
      {
        Beam::deleteIfUnused(_ppiBeams[i]);
	_ppiBeams.erase(_ppiBeams.begin()+i);
      }
    }

  } /* endif - need_to_cull */
  
}
#endif

/*************************************************************************
 * _refreshImages()
 */

void HorizWidget::_refreshImages()
{

#ifdef NOTNOW  
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
    
    // Add pointers to the beams to be rendered
    
    if (ifield == _selectedField || field->isBackgroundRendered()) {

      std::vector< PpiBeam* >::iterator beam;
      for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
	(*beam)->setBeingRendered(ifield, true);
	field->addBeam(*beam);
      }
      
    }
    
  } // ifield
#endif
  
  // do the rendering

  _performRendering();

  update();
}

void HorizWidget::contextMenuParameterColors()
{

#ifdef NOTNOW
  
  LOG(DEBUG_VERBOSE) << "enter";

  //DisplayField selectedField;                                                                             

  // const DisplayField &field = _manager.getSelectedField();
  // const ColorMap &colorMapForSelectedField = field.getColorMap();
  ParameterColorView *parameterColorView = new ParameterColorView(this);
  vector<DisplayField *> displayFields = _manager.getDisplayFields(); // TODO: I guess, implement this as a signal and a slot? // getDisplayFields();
  DisplayField selectedField = _manager.getSelectedField();
  string emphasis_color = "white";
  string annotation_color = "white";
  DisplayFieldModel *displayFieldModel = 
    new DisplayFieldModel(displayFields, selectedField.getName(),
			  _params.grid_and_range_ring_color,
			  emphasis_color,
			  annotation_color,
			  _params.background_color);
  FieldColorController *fieldColorController = new FieldColorController(parameterColorView, displayFieldModel);
  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  //  connect(parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
  connect(fieldColorController, SIGNAL(colorMapRedefineSent(string, ColorMap, QColor, QColor, QColor, QColor)),
  	  &_manager, SLOT(colorMapRedefineReceived(string, ColorMap, QColor, QColor, QColor, QColor))); // THIS IS NOT CALLED!!
  //  CartManager::colorMapRedefineReceived(string, ColorMap)
  //connect(fieldColorController, SIGNAL(colorMapRedefined(string)),
  //	  this, SLOT(changeToDisplayField(string))); // THIS IS NOT CALLED!!

  /* TODO: combine with replot
  connect(fieldColorController, SIGNAL(backgroundColorSet(QColor)),
  	  this, SLOT(backgroundColor(QColor)));
  */

  fieldColorController->startUp(); 

  //connect(parameterColorView, SIGNAL(needFieldNames()), this, SLOT(getFieldNames()));
  //connect(this, SIGNAL(fieldNamesSupplied(vector<string>)), 
  //  parameterColorView, SLOT(fieldNamesSupplied(vector<string>));
  // TODO: move this call to the controller?                                                                
	  // parameterColorView.exec();

  //  if(parameterColorController.Changes()) {
    // TODO: what are changes?  new displayField(s)?                                                        
  //}
 
  LOG(DEBUG_VERBOSE) << "exit ";

#endif
  
}

/*
void HorizWidget::sillyReceived() {
  LOG(DEBUG_VERBOSE) << "enter";
  LOG(DEBUG_VERBOSE) << "exit";
}
*/
/*
void HorizWidget::changeToDisplayField(string fieldName)  // , ColorMap newColorMap) {
{
  LOG(DEBUG_VERBOSE) << "enter";
  // connect the new color map with the field                                                                    
  // find the fieldName in the list of FieldDisplays                                                             
  
  bool found = false;
  vector<DisplayField *>::iterator it;
  int fieldId = 0;

  it = _fields.begin();
  while ( it != _fields.end() && !found ) {
    DisplayField *field = *it;

    string name = field->getName();
    if (name.compare(fieldName) == 0) {
      found = true;
      field->replaceColorMap(newColorMap);
    }
    fieldId++;
    it++;
  }
  if (!found) {
    LOG(ERROR) << fieldName;
    LOG(ERROR) << "ERROR - field not found; no color map change";
    // TODO: present error message box                                                                           
  } else {
    // look up the fieldId from the fieldName                                                                    
    // change the field variable                                                                                 
    _changeField(fieldId, true);
  }
  
  LOG(DEBUG_VERBOSE) << "exit";
}
*/


#ifdef NOTNOW

void HorizWidget::ExamineEdit(const RadxRay *closestRay) {
  

  // get an version of the ray that we can edit
  // we'll need the az, and sweep number to get a list from
  // the volume

  vector<RadxRay *> rays = _vol->getRays();
  // find that ray
  bool foundIt = false;
  RadxRay *closestRayToEdit = NULL;
  vector<RadxRay *>::iterator r;
  r=rays.begin();
  int idx = 0;
  while(r<rays.end()) {
    RadxRay *rayr = *r;
    if (closestRay->getAzimuthDeg() == rayr->getAzimuthDeg()) {
      if (closestRay->getElevationDeg() == rayr->getElevationDeg()) {
        foundIt = true;
        closestRayToEdit = *r;
        LOG(DEBUG_VERBOSE) << "Found closest ray: index = " << idx << " pointer = " << closestRayToEdit;
        closestRay->print(cout); 
      }
    }
    r += 1;
    idx += 1;
  }  
  if (!foundIt || closestRayToEdit == NULL)
    throw "couldn't find closest ray";

  
  //RadxRay *closestRayCopy = new RadxRay(*closestRay);

  // create the view
  SpreadSheetView *sheetView;
  sheetView = new SpreadSheetView(this, closestRayToEdit->getAzimuthDeg());

  // create the model

  // SpreadSheetModel *model = new SpreadSheetModel(closestRayCopy);
  SpreadSheetModel *model = new SpreadSheetModel(closestRayToEdit, _vol);
  //SpreadSheetModel *model = new SpreadSheetModel(closestRay, _vol);
  
  // create the controller
  SpreadSheetController *sheetControl = new SpreadSheetController(sheetView, model);

  // finish the other connections ..
  //sheetView->addController(sheetController);
  // model->setController(sheetController);

  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  connect(sheetControl, SIGNAL(volumeChanged()),
  	  &_manager, SLOT(setVolume()));
  
  sheetView->init();
  sheetView->show();
  sheetView->layout()->setSizeConstraint(QLayout::SetFixedSize);
  
}
#endif

void HorizWidget::contextMenuEditor()
{
  LOG(DEBUG_VERBOSE) << "enter";

  // get click location in world coords
  // by using the location stored in class variables
  double x_km = _worldPressX;
  double y_km = _worldPressY;

  // get ray closest to click point
  const RadxRay *closestRay = _getClosestRay(x_km, y_km);
  // TODO: make sure the point is in the valid area
  if (closestRay == NULL) {
    // report error
    QMessageBox::information(this, QString::fromStdString(""), QString::fromStdString("No ray found at location clicked"));
    // TODO: move to this ...  errorMessage("", "No ray found at location clicked");
  } else {
    // ExamineEdit(closestRay);
  }
  LOG(DEBUG_VERBOSE) << "exit";
}

void HorizWidget::ShowContextMenu(const QPoint &pos/* , RadxVol *vol */)
{

  // _vol = vol;

  QMenu contextMenu("Context menu", this);
  
  QAction action1("Cancel", this);
  connect(&action1, SIGNAL(triggered()), this, SLOT(contextMenuCancel()));
  contextMenu.addAction(&action1);

  QAction action3("Parameters + Colors", this);
  connect(&action3, SIGNAL(triggered()), this, SLOT(contextMenuParameterColors()));
  contextMenu.addAction(&action3);

  QAction action4("View", this);
  connect(&action4, SIGNAL(triggered()), this, SLOT(contextMenuView()));
  contextMenu.addAction(&action4);

  QAction action5("Editor", this);
  connect(&action5, SIGNAL(triggered()), this, SLOT(contextMenuEditor()));
  contextMenu.addAction(&action5);
  
  QAction action6("Examine", this);
  connect(&action6, SIGNAL(triggered()), this, SLOT(contextMenuExamine()));
  contextMenu.addAction(&action6);

  /*
  QAction action7("Data Widget", this);
  connect(&action7, SIGNAL(triggered()), this, SLOT(contextMenuDataWidget()));
  contextMenu.addAction(&action7);
  */

  contextMenu.exec(this->mapToGlobal(pos));
}

/*************************************************************************
 * react to click point from remote display - Sprite
 * redraw the click point cursor
 */

void HorizWidget::setClickPoint(double azimuthDeg,
                              double elevationDeg,
                              double rangeKm)
{

  double x_km =
    rangeKm * sin(azimuthDeg * DEG_TO_RAD) * cos(elevationDeg * DEG_TO_RAD);
  double y_km =
    rangeKm * cos(azimuthDeg * DEG_TO_RAD) * cos(elevationDeg * DEG_TO_RAD);

  _mouseReleaseX = _zoomWorld.getIxPixel(x_km);
  _mouseReleaseY = _zoomWorld.getIyPixel(y_km);
  _pointClicked = true;

  update();

}
