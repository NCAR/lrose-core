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
#include <QToolTip>

#include "PolarWidget.hh"
#include "PolarManager.hh"

using namespace std;


const double PolarWidget::SIN_45 = sin(45.0 * DEG_TO_RAD);
const double PolarWidget::SIN_30 = sin(30.0 * DEG_TO_RAD);
const double PolarWidget::COS_30 = cos(30.0 * DEG_TO_RAD);

PolarWidget::PolarWidget(QWidget* parent,
                         const PolarManager &manager,
                         const Params &params,
                         const RadxPlatform &platform,
                         const vector<DisplayField *> &fields,
                         bool haveFilteredFields) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(params),
        _platform(platform),
        _fields(fields),
        _haveFilteredFields(haveFilteredFields),
        _selectedField(0),
        _backgroundBrush(QColor(_params.background_color)),
        _gridRingsColor(_params.grid_and_range_ring_color),
        _ringsEnabled(false),
        _gridsEnabled(false),
        _angleLinesEnabled(false),
        _scaledLabel(ScaledLabel::DistanceEng),
        _rubberBand(0),
        _ringSpacing(10.0)

{

  // mode

  _archiveMode = _params.begin_in_archive_mode;

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

  // Delete all of the field renderers

  for (size_t i = 0; i < _fieldRenderers.size(); ++i) {
    delete _fieldRenderers[i];
  }
  _fieldRenderers.clear();

}


/*************************************************************************
 * configure the axes
 */

/*************************************************************************
 * set archive mode
 */

void PolarWidget::setArchiveMode(bool archive_mode)
{
  _archiveMode = archive_mode;
}

/*************************************************************************
 * unzoom the view
 */

void PolarWidget::unzoomView()
{
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();
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
 * setAngleLines()
 */

void PolarWidget::setAngleLines(const bool enabled)
{
  _angleLinesEnabled = enabled;
  update();
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


    /***** testing ******
    // QToolTip::showText(mapToGlobal(QPoint(_mouseReleaseX, _mouseReleaseY)), "louigi")  
    QToolTip::showText(QPoint(0,0), "louigi");

    smartBrush(_mouseReleaseX, _mouseReleaseY);

    // ***** end testing ****/

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

/**************   testing ******/
void PolarWidget::smartBrush(int xPixel, int yPixel) {
  //int xp = _ppi->_zoomWorld.getIxPixel(xkm);
  //int yp = _ppi->_zoomWorld.getIyPixel(ykm);

  QImage qImage;
  qImage.load("/h/eol/brenda/octopus.jpg");
  // get the Image from somewhere ...   
  //qImage->convertToFormat(QImage::Format_RGB32);
  //qImage->invertPixels();
  QPainter painter(this);
  painter.drawImage(0, 0, qImage);
  _drawOverlays(painter);

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
  _setGridSpacing();

}


/*************************************************************************
 * Protected methods
 *************************************************************************/

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

