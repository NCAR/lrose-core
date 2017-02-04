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
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/uusleep.h>

#include <qtimer.h>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "PolarWidget.hh"
#include "PolarManager.hh"

using namespace std;


const double PolarWidget::SIN_45 = sin(45.0 * DEG_TO_RAD);
const double PolarWidget::SIN_30 = sin(30.0 * DEG_TO_RAD);
const double PolarWidget::COS_30 = cos(30.0 * DEG_TO_RAD);

PolarWidget::PolarWidget(QWidget* parent,
                         const PolarManager &manager,
                         const Params &params,
                         size_t n_fields) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(params),
        _nFields(n_fields),
        _selectedField(0),
        _gridRingsColor(_params.grid_and_range_ring_color),
        _backgroundBrush(QColor(_params.background_color)),
        _ringsEnabled(true),
        _gridsEnabled(false),
        _azLinesEnabled(true),
        _scaledLabel(ScaledLabel::DistanceEng),
        _rubberBand(0),
        _ringSpacing(10.0)

{

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
  
  for (size_t i = 0; i < _nFields; ++i) {
    FieldRenderer *field = new FieldRenderer(_params, i);
    field->createImage(width(), height());
    _fieldRenderers.push_back(field);
  }

  // init other values

  _worldPressX = 0.0;
  _worldPressY = 0.0;
  _worldReleaseX = 0.0;
  _worldReleaseY = 0.0;
  _mousePressX = 0;
  _mousePressY = 0;
  _mouseReleaseX = 0;
  _mouseReleaseY = 0;
  _zoomCornerX = 0;
  _zoomCornerY = 0;
  
}


/*************************************************************************
 * Destructor
 */

PolarWidget::~PolarWidget()
{
  // delete all of the dynamically created beams

  for (size_t i = 0; i < _beams.size(); ++i) {
    delete _beams[i];
  }
  _beams.clear();

  // Delete all of the field renderers

  for (size_t i = 0; i < _fieldRenderers.size(); ++i) {
    delete _fieldRenderers[i];
  }
  _fieldRenderers.clear();

}


/*************************************************************************
 * unzoom the view
 */

void PolarWidget::unzoomView()
{
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setRingSpacing();
  _refreshImages();
}


/*************************************************************************
 * clear()
 */

void PolarWidget::clear()
{
  // Clear out the beam array

  for (size_t i = 0; i < _beams.size(); i++) {
    delete _beams[i];
  }
  _beams.clear();
  
  // Now rerender the images
  
  _refreshImages();

}


/*************************************************************************
 * setRings()
 */

void PolarWidget::setRings(const bool enabled)
{
  _ringsEnabled = enabled;
  update();
}


/*************************************************************************
 * setGrids()
 */

void PolarWidget::setGrids(const bool enabled)
{
  _gridsEnabled = enabled;
  update();
}


/*************************************************************************
 * setAzLines()
 */

void PolarWidget::setAzLines(const bool enabled)
{
  _azLinesEnabled = enabled;
  update();
}


/*************************************************************************
 * numBeams()
 */

int PolarWidget::numBeams() const
{
  return _beams.size();
}


/*************************************************************************
 * selectVar()
 */

void PolarWidget::selectVar(const size_t index)
{

  // If the field index isn't actually changing, we don't need to do anything
  
  if (_selectedField == index) {
    return;
  }
  
  // If this field isn't being rendered in the background, render all of
  // the beams for it

  if (!_fieldRenderers[index]->isBackgroundRendered()) {
    std::vector< PolarBeam* >::iterator beam;
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

void PolarWidget::clearVar(const size_t index)
{

  if (index >= _nFields) {
    return;
  }

  // Set the brush for every beam/gate for this field to use the background
  // color

  std::vector< PolarBeam* >::iterator beam;
  for (beam = _beams.begin(); beam != _beams.end(); ++beam) {
    (*beam)->resetFieldBrush(index, &_backgroundBrush);
  }
  
  if (index == _selectedField) {
    update();
  }

}


/*************************************************************************
 * turn on archive-style rendering - all fields
 */

void PolarWidget::activateArchiveRendering()
{
  for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
    _fieldRenderers[ii]->setBackgroundRenderingOn();
  }
}


/*************************************************************************
 * turn on reatlime-style rendering - non-selected fields in background
 */

void PolarWidget::activateRealtimeRendering()
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

void PolarWidget::addBeam(const RadxRay *ray,
                          const float start_angle,
                          const float stop_angle,
                          const std::vector< std::vector< double > > &beam_data,
                          const std::vector< DisplayField* > &fields)
{
  // add a new beam to the display. 
  // The steps are:
  // 1. preallocate mode: find the beam to be drawn, or dynamic mode:
  //    create the beam(s) to be drawn.
  // 2. fill the colors for all variables in the beams to be drawn
  // 3. make the display list for the selected variables in the beams
  //    to be drawn.
  // 4. call the new display list(s)

  std::vector< PolarBeam* > newBeams;

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

  float n_start_angle = start_angle - ((int)(start_angle/360.0))*360.0;
  float n_stop_angle = stop_angle - ((int)(stop_angle/360.0))*360.0;
  
  if (n_start_angle <= n_stop_angle) {

    // This beam does not cross the 0 degree angle.  Just add the beam to
    // the beam list.

    PolarBeam* b = new PolarBeam(_params, ray, _nFields, n_start_angle, n_stop_angle);
    _cullBeams(b);
    _beams.push_back(b);
    newBeams.push_back(b);

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    PolarBeam* b1 = new PolarBeam(_params, ray, _nFields, n_start_angle, 360.0);
    _cullBeams(b1);
    _beams.push_back(b1);
    newBeams.push_back(b1);

    // Now add the portion of the beam to the right of the 0 degree point.

    PolarBeam* b2 = new PolarBeam(_params, ray, _nFields, 0.0, n_stop_angle);
    _cullBeams(b2);
    _beams.push_back(b2);
    newBeams.push_back(b2);

  }

  // newBeams has pointers to all of the newly added beams.  Render the
  // beam data.

  for (size_t ii = 0; ii < newBeams.size(); ii++) {

    PolarBeam *beam = newBeams[ii];
    
    // Set up the brushes for all of the fields in this beam.  This can be
    // done independently of a Painter object.
    
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
    
  } /* endfor - beam */
  
  // Start the threads to render the new beams

  _performRendering();

}


/*************************************************************************
 * displayImage()
 */

void PolarWidget::displayImage(const size_t field_num)
{
  // If we weren't rendering the current field, do nothing
  if (field_num != _selectedField) {
    return;
  }
  update();
}


/*************************************************************************
 * backgroundColor()
 */

void PolarWidget::backgroundColor(const QColor &color)
{
  _backgroundBrush.setColor(color);
  QPalette new_palette = palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  setPalette(new_palette);
  _refreshImages();
}


/*************************************************************************
 * gridRingsColor()
 */

void PolarWidget::gridRingsColor(const QColor &color)
{
  _gridRingsColor = color;
  update();
}


/*************************************************************************
 * getImage()
 */

QImage* PolarWidget::getImage()
{
  QPixmap pixmap = QPixmap::grabWidget(this);
  QImage* image = new QImage(pixmap.toImage());
  return image;
}


/*************************************************************************
 * getPixmap()
 */

QPixmap* PolarWidget::getPixmap()
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

void PolarWidget::mousePressEvent(QMouseEvent *e)
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

void PolarWidget::mouseMoveEvent(QMouseEvent * e)
{

  // Zooming with the mouse

  int x = e->x();
  int y = e->y();
  int deltaX = x - _mousePressX;
  int deltaY = y - _mousePressY;

  // Make the rubberband aspect ratio match that
  // of the window

  double dx = fabs(deltaY * _aspectRatio);
  double dy = fabs(dx / _aspectRatio);

  // Preserve the signs

  dx *= fabs(deltaX)/deltaX;
  dy *= fabs(deltaY)/deltaY;

  int moveX = (int) floor(dx + 0.5);
  int moveY = (int) floor(dy + 0.5);
  QRect newRect = QRect(_mousePressX, _mousePressY, moveX, moveY);

  _zoomCornerX = _mousePressX + moveX;
  _zoomCornerY = _mousePressY + moveY;

  newRect = newRect.normalized();
  _rubberBand->setGeometry(newRect);

}


/*************************************************************************
 * mouseReleaseEvent()
 */

void PolarWidget::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  QRect rgeom = _rubberBand->geometry();

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

  QPointF clickPos(e->pos());
  
  _mouseReleaseX = clickPos.x();
  _mouseReleaseY = clickPos.y();

  // get click location in world coords

  if (rgeom.width() <= 20) {
    
    // Emit a signal to indicate that the click location has changed
    
    _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
    _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

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

    _zoomWorld.set(_worldPressX, _worldPressY, _worldReleaseX, _worldReleaseY);

    _setTransform(_zoomWorld.getTransform());

    _setRingSpacing();

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

void PolarWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  painter.drawImage(0, 0, *(_fieldRenderers[_selectedField]->getImage()));

  _drawOverlays(painter);
}


/*************************************************************************
 * resizeEvent()
 */

void PolarWidget::resizeEvent(QResizeEvent * e)
{
  _resetWorld(width(), height());
  _refreshImages();
  update();
}


/*************************************************************************
 * resize()
 */

void PolarWidget::resize(const int width, const int height)
{

  // Set the geometry based on the aspect ratio that we need for this display.
  // The setGeometry() method will fire off the resizeEvent() so we leave the
  // updating of the display to that event.
  
  int sizeNeeded = (int) ((width - _colorScaleWidth) / _aspectRatio + 0.5);
  if (height < sizeNeeded) {
    sizeNeeded = height;
  }

  setGeometry(0, 0, 
              (int) (sizeNeeded * _aspectRatio + 0.5) + _colorScaleWidth,
              sizeNeeded);

}

//////////////////////////////////////////////////////////////
// reset the pixel size of the world view

void PolarWidget::_resetWorld(int width, int height)

{

  _fullWorld.resize(width, height);
  _zoomWorld = _fullWorld;
  _setTransform(_fullWorld.getTransform());
  _setRingSpacing();

}


/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * _beamIndex()
 */

int PolarWidget::_beamIndex(const double start_angle,
			   const double stop_angle)
{
  // Find where the center angle of the beam will fall within the beam array

  int i = (int)(_beams.size()*(start_angle + (stop_angle-start_angle)/2)/360.0);

  // Take care of the cases at the ends of the beam list

  if (i < 0)
    i = 0;
  if (i > (int)_beams.size() - 1)
    i = _beams.size() - 1;

  return i;
}


/*************************************************************************
 * _cullBeams()
 */

void PolarWidget::_cullBeams(const PolarBeam *beamAB)
{
  // This routine examines the collection of beams, and removes those that are 
  // completely occluded by other beams. The algorithm gives precedence to the 
  // most recent beams; i.e. beams at the end of the _beams vector.
  //
  // Remember that there won't be any beams that cross angles through zero; 
  // otherwise the beam culling logic would be a real pain, and PolarWidget has
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
  // If an overlap on beam XY is detected, the occluded region is recorded as the interval (CD):        
  //         a---------b
  //    x---------y
  //         c----d
  // 
  // The culling algorithm starts with the last beam in the list, and compares it with all
  // preceeding beams, setting their overlap regions appropriately. Then the next to the last
  // beam is compared with all preceeding beams. Previously found occluded regions will be 
  // expanded as they are detected.
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

  if (_beams.size() < 1)
    return;

  // Look through all of the beams in the list and record any place where
  // this beam occludes any other beam.

  bool need_to_cull = false;
  
  // Save the angle information for easier processing.
  
  double a = beamAB->startAngle;
  double b = beamAB->stopAngle;

  // Look at all of the beams in the list to see if any are occluded by this
  // new beam

  for (size_t j = 0; j < _beams.size(); ++j)
  {
    // Pull the beam from the list for ease of coding

    PolarBeam *beamXY = _beams[j];

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

    for (int i = _beams.size()-1; i >= 0; i--)
    {
      // Delete beams who are hidden but aren't currently being rendered.
      // We can get the case where we have hidden beams that are being
      // rendered when we do something (like resizing) that causes us to 
      // have to rerender all of the current beams.  During the rerendering,
      // new beams continue to come in and will obscure some of the beams
      // that are still in the rendering queue.  These beams will be deleted
      // during a later pass through this loop.

      if (_beams[i]->hidden && !_beams[i]->isBeingRendered())
      {
	delete _beams[i];
	_beams.erase(_beams.begin()+i);
      }
    }

  } /* endif - need_to_cull */
  
}


/*************************************************************************
 * _drawOverlays()
 */

void PolarWidget::_drawOverlays(QPainter &painter)
{

  // Don't try to draw rings if we haven't been configured yet or if the
  // rings or grids aren't enabled.
  
  if (!_ringsEnabled && !_gridsEnabled && !_azLinesEnabled) {
    return;
  }

  // save painter state

  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  // Set the painter to use the right color and font

  // painter.setWindow(_zoomWindow);
  
  painter.setPen(_gridRingsColor);

  // Draw rings

  if (_ringSpacing > 0.0 && _ringsEnabled) {

    // Draw the rings

    painter.save();
    painter.setTransform(_zoomTransform);
    double ringRange = _ringSpacing;
    while (ringRange <= _maxRange) {
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
    while (ringRange <= _maxRange) {
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

    double ringRange = _ringSpacing;
    double maxRingRange = ringRange;
    while (ringRange <= _maxRange) {

      _zoomWorld.drawLine(painter, ringRange, -_maxRange, ringRange, _maxRange);
      _zoomWorld.drawLine(painter, -ringRange, -_maxRange, -ringRange, _maxRange);
      _zoomWorld.drawLine(painter, -_maxRange, ringRange, _maxRange, ringRange);
      _zoomWorld.drawLine(painter, -_maxRange, -ringRange, _maxRange, -ringRange);
      
      maxRingRange = ringRange;
      ringRange += _ringSpacing;
    }

    _zoomWorld.setSpecifyTicks(true, -maxRingRange, _ringSpacing);

    _zoomWorld.drawAxisLeft(painter, "km", true, true, true);
    _zoomWorld.drawAxisRight(painter, "km", true, true, true);
    _zoomWorld.drawAxisTop(painter, "km", true, true, true);
    _zoomWorld.drawAxisBottom(painter, "km", true, true, true);
    
    _zoomWorld.setSpecifyTicks(false);

  }
  
  // Draw the azimuth lines

  if (_azLinesEnabled) {

    // Draw the lines along the X and Y axes

    _zoomWorld.drawLine(painter, 0, -_maxRange, 0, _maxRange);
    _zoomWorld.drawLine(painter, -_maxRange, 0, _maxRange, 0);

    // Draw the lines along the 30 degree lines

    double end_pos1 = SIN_30 * _maxRange;
    double end_pos2 = COS_30 * _maxRange;
    
    _zoomWorld.drawLine(painter, end_pos1, end_pos2, -end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, end_pos1, -end_pos2, -end_pos1);
    _zoomWorld.drawLine(painter, -end_pos1, end_pos2, end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, -end_pos1, -end_pos2, end_pos1);

  }
  
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
  _zoomWorld.drawColorScale(field.getColorMap(), painter);
  
}


/*************************************************************************
 * _refreshImages()
 */

void PolarWidget::_refreshImages()
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
    
    // Add pointers to the beams to be rendered
    
    if (ifield == _selectedField || field->isBackgroundRendered()) {

      std::vector< PolarBeam* >::iterator beam;
      for (beam = _beams.begin(); beam != _beams.end(); ++beam) {
	(*beam)->setBeingRendered(ifield, true);
	field->addBeam(*beam);
      }
      
    }
    
  } // ifield
  
  // do the rendering

  _performRendering();

  update();
}


/*************************************************************************
 * _setRingSpacing()
 */

void PolarWidget::_setRingSpacing()
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
  } else {
    _ringSpacing = 20.0;
  }

}


////////////////////
// set the transform

void PolarWidget::_setTransform(const QTransform &transform)
{
  
  _fullTransform = transform;
  _zoomTransform = transform;
  
}
  
/*************************************************************************
 * perform the rendering
 */

void PolarWidget::_performRendering()
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

