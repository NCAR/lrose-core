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
#include <toolsa/LogStream.hh>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QErrorMessage>

// #include "ParameterColorView.hh"
// #include "DisplayFieldModel.hh"
// #include "FieldColorController.hh"
#include "CartWidget.hh"
#include "CartManager.hh"
// #include "SpreadSheetView.hh"
// #include "SpreadSheetController.hh"
// #include "BoundaryPointEditor.hh"

#include "cidd.h"

using namespace std;


const double CartWidget::SIN_45 = sin(45.0 * DEG_TO_RAD);
const double CartWidget::SIN_30 = sin(30.0 * DEG_TO_RAD);
const double CartWidget::COS_30 = cos(30.0 * DEG_TO_RAD);

CartWidget::CartWidget(QWidget* parent,
                       const CartManager &manager) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
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

  _archiveMode = _params.start_mode == Params::MODE_ARCHIVE;

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
  
  // for (size_t ii = 0; ii < _fields.size(); ii++) {
  //   FieldRenderer *fieldRenderer =
  //     new FieldRenderer(_params, ii, *_fields[ii]);
  //   fieldRenderer->createImage(width(), height());
  //   _fieldRenderers.push_back(fieldRenderer);
  // }

  // init other values

  _worldPressX = 0.0;
  _worldPressY = 0.0;
  _worldReleaseX = 0.0;
  _worldReleaseY = 0.0;
  _pointClicked = false;
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

CartWidget::~CartWidget()
{

  // Delete all of the field renderers

  // for (size_t i = 0; i < _fieldRenderers.size(); ++i) {
  //   delete _fieldRenderers[i];
  // }
  // _fieldRenderers.clear();

}


/*************************************************************************
 * configure the axes
 */

/*************************************************************************
 * set archive mode
 */

void CartWidget::setArchiveMode(bool archive_mode)
{
  _archiveMode = archive_mode;
}

/*************************************************************************
 * unzoom the view
 */

void CartWidget::unzoomView()
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

void CartWidget::setRings(const bool enabled)
{
  _ringsEnabled = enabled;
  update();
}


/*************************************************************************
 * setGrids()
 */

void CartWidget::setGrids(const bool enabled)
{
  _gridsEnabled = enabled;
  update();
}


/*************************************************************************
 * setAngleLines()
 */

void CartWidget::setAngleLines(const bool enabled)
{
  _angleLinesEnabled = enabled;
  update();
}


/*************************************************************************
 * turn on archive-style rendering - all fields
 */

void CartWidget::activateArchiveRendering()
{
  // for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
  //   _fieldRenderers[ii]->setBackgroundRenderingOn();
  // }
}


/*************************************************************************
 * turn on reatlime-style rendering - non-selected fields in background
 */

void CartWidget::activateRealtimeRendering()
{
  
  // for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
  //   if (ii != _selectedField) {
  //     _fieldRenderers[ii]->activateBackgroundRendering();
  //   }
  // }

}

/*************************************************************************
 * displayImage()
 */

void CartWidget::displayImage(const size_t field_num)
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

void CartWidget::backgroundColor(const QColor &color)
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

void CartWidget::gridRingsColor(const QColor &color)
{
  LOG(DEBUG_VERBOSE) << "enter " << color.name().toStdString();
  _gridRingsColor = color;
  update();
  LOG(DEBUG_VERBOSE) << "exit";
}


/*************************************************************************
 * getImage()
 */

QImage* CartWidget::getImage()
{
  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;
}


/*************************************************************************
 * getPixmap()
 */

QPixmap* CartWidget::getPixmap()
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

void CartWidget::mousePressEvent(QMouseEvent *e)
{

  cerr << "cccc mousePressEvent" << endl;
  
#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  if (e->button() == Qt::RightButton) {
    
    //-------

    _mousePressX = pos.x();
    _mousePressY = pos.y();

    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    emit customContextMenuRequested(pos.toPoint()); // , closestRay);

  } else {


    _rubberBand->setGeometry(pos.x(), pos.y(), 0, 0);
    _rubberBand->show();

    _mousePressX = pos.x();
    _mousePressY = pos.y();

    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);
  }
}


/*************************************************************************
 * mouseMoveEvent(), mouse button is down and mouse is moving
 */

void CartWidget::mouseMoveEvent(QMouseEvent * e)
{

  cerr << "ccccc mouseMoveEvent" << endl;
  
  // Zooming with the mouse

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  int x = pos.x();
  int y = pos.y();
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

void CartWidget::mouseReleaseEvent(QMouseEvent *e)
{

  cerr << "ccccc mouseReleaseEvent" << endl;

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  _pointClicked = false;

  if (e->button() == Qt::RightButton) {

    _mousePressX = pos.x();
    _mousePressY = pos.y();

    emit customContextMenuRequested(pos.toPoint()); // , closestRay);

  } else {
    
    QRect rgeom = _rubberBand->geometry();

    // If the mouse hasn't moved much, assume we are clicking rather than
    // zooming

    _mouseReleaseX = pos.x();
    _mouseReleaseY = pos.y();

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
}

#ifdef NOTNOW
/**************   testing ******/

void CartWidget::smartBrush(int xPixel, int yPixel) 
{

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
#endif

/*************************************************************************
 * paintEvent()
 */

void CartWidget::paintEvent(QPaintEvent *event)
{

  cerr << "paintEvent" << endl;
  
  QPainter painter(this);

  // painter.drawImage(0, 0, *(_fieldRenderers[_selectedField]->getImage()));

  _drawOverlays(painter);

  //if there are no points, this does nothing
  // BoundaryPointEditor::Instance()->draw(_zoomWorld, painter);

}


/*************************************************************************
 * resizeEvent()
 */

void CartWidget::resizeEvent(QResizeEvent * e)
{
  _resetWorld(width(), height());
  _refreshImages();
  update();
}


/*************************************************************************
 * resize()
 */

void CartWidget::resize(const int width, const int height)
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

void CartWidget::_resetWorld(int width, int height)

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

void CartWidget::_setTransform(const QTransform &transform)
{
  // float worldScale = _zoomWorld.getXMaxWindow() - _zoomWorld.getXMinWindow();
  // BoundaryPointEditor::Instance()->setWorldScale(worldScale);

  _fullTransform = transform;
  _zoomTransform = transform;
}
  
/*************************************************************************
 * perform the rendering
 */

void CartWidget::_performRendering()
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

void CartWidget::informationMessage()
{
  
  // QMessageBox::StandardButton reply;
  // QLabel *informationLabel;
  
  // reply = QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
  QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
  //  if (reply == QMessageBox::Ok)
  //  informationLabel->setText("OK");
  //else
  //  informationLabel->setText("Escape");

}

// void CartWidget::notImplemented()
// {
//   cerr << "inside notImplemented() ... " << endl;

//   QErrorMessage *errorMessageDialog = new QErrorMessage(_parent);
//   // QLabel *informationLabel = new QLabel();

//   errorMessageDialog->showMessage("This option is not implemented yet.");
//   QLabel errorLabel;
//   int frameStyle = QFrame::Sunken | QFrame::Panel;
//   errorLabel.setFrameStyle(frameStyle);
//   errorLabel.setText("If the box is unchecked, the message "
// 		     "won't appear again.");

//   cerr << "exiting notImplemented() " << endl;

// }


// slots for context editing; create and show the associated modeless dialog and return                                   

void CartWidget::contextMenuCancel()
{
  // informationMessage();
  // notImplemented();                                                                                                     
}

void CartWidget::contextMenuParameterColors()
{
  /*
    LOG(DEBUG_VERBOSE) << "enter";

    //DisplayField selectedField;

    const DisplayField &field = _manager.getSelectedField();
    const ColorMap &colorMapForSelectedField = field.getColorMap();
    ParameterColorView *parameterColorView = new ParameterColorView(this);
    vector<DisplayField> displayFields = _manager.getDisplayFields();
    DisplayFieldModel *displayFieldModel = new DisplayFieldModel(displayFields);
    FieldColorController fieldColorController(parameterColorView, displayFieldModel);
    // connect some signals and slots in order to retrieve information
    // and send changes back to display 
    connect(&parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
    connect(&parameterColorView, SIGNAL(changesToDisplay()), &_manager, SLOT(changesToDisplayFields()));

    // TODO: move this call to the controller?
    parameterColorView.exec();

    if(parameterColorController.Changes()) {
    // TODO: what are changes?  new displayField(s)?
    }
  
    // TODO: where to delete the ParameterColor objects & disconnect the signals and slots??
    delete parameterColorView;
    delete parameterColorModel;

    LOG(DEBUG_VERBOSE) << "exit ";
  */
  informationMessage();
   
}

void CartWidget::contextMenuView()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}

void CartWidget::contextMenuEditor()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}


void CartWidget::contextMenuExamine()         
{
  informationMessage();                                                                                                 

}

void CartWidget::contextMenuDataWidget()
{
  informationMessage();

  //  notImplemented();                                                                                                   
}

void CartWidget::ShowContextMenu(const QPoint &pos, RadxVol *vol)
{

}


