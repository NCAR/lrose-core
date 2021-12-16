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

#include "ParameterColorView.hh"
#include "DisplayFieldModel.hh"
#include "FieldColorController.hh"
#include "PolarWidget.hh"
#include "PolarManager.hh"
#include "SpreadSheetView.hh"
#include "SpreadSheetController.hh"
#include "BoundaryPointEditor.hh"

using namespace std;


const double PolarWidget::SIN_45 = sin(45.0 * DEG_TO_RAD);
const double PolarWidget::SIN_30 = sin(30.0 * DEG_TO_RAD);
const double PolarWidget::COS_30 = cos(30.0 * DEG_TO_RAD);

PolarWidget::PolarWidget(QWidget* parent,
                        PolarManager *manager,
                         const RadxPlatform &platform,
			 DisplayFieldController *displayFieldController,
			 //                         const vector<DisplayField *> &fields,
                         bool haveFilteredFields,
                         RayLocationController *rayLocationController) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _platform(platform),
        //_fields(fields),
	      displayFieldController(displayFieldController),
        _haveFilteredFields(haveFilteredFields),
        // _selectedField(0),
        _ringsEnabled(false),
        _gridsEnabled(false),
        _angleLinesEnabled(false),
        _scaledLabel(ScaledLabel::DistanceEng),
        _rubberBand(0),
        _ringSpacing(10.0),
        _boundaryTrackMouseMove(false),
        _rayLocationController(rayLocationController)

{
  _params = ParamFile::Instance();
  string color = _params->backgroundColor;
  _backgroundBrush = QColor(color.c_str());
  _gridRingsColor = _params->gridColor.c_str();

  _aspectRatio = _params->ppi_aspect_ratio;
  _colorScaleWidth = _params->color_scale_width;

  setRings(_params->ppi_range_rings_on_at_startup);
  setGrids(_params->ppi_grids_on_at_startup);
  setAngleLines(_params->ppi_azimuth_lines_on_at_startup);

  // mode

//  _archiveMode = _params->begin_in_archive_mode;

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
  //_rubberBandLine = new QRubberBand(QRubberBand::Line, this);

  // Allow the size_t type to be passed to slots

  qRegisterMetaType<size_t>("size_t");

  _resetWorld(_manager->height(), _manager->height());

  // create the field renderers
  _fieldRendererController = new FieldRendererController();

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

PolarWidget::~PolarWidget()
{

}


void PolarWidget::configureRange(double max_range)
{
  LOG(DEBUG) << "enter max_range = " << max_range;
  // Save the specified values

  _maxRangeKm = max_range;

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setGridSpacing();
  
  // set world view

  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  int colorScaleWidth = _params->color_scale_width;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;

  if (_params->ppi_display_type == Params::PPI_AIRBORNE) {

    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   // xMinWorld, yMinWorld
                   -_maxRangeKm, 0.0,
                   // xMaxWorld, yMaxWorld
                   _maxRangeKm, _maxRangeKm,
                   axisTickLen, nTicksIdeal, textMargin);
    
  } else {
    
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   // xMinWorld, yMinWorld
                   -_maxRangeKm, -_maxRangeKm,
                   // xMaxWorld, yMaxWorld
                   _maxRangeKm, _maxRangeKm,
                   axisTickLen, nTicksIdeal, textMargin);

  }
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();

  // Initialize the images used for double-buffering.  For some reason,
  // the window size is incorrect at this point, but that will be corrected
  // by the system with a call to resize().
  LOG(DEBUG) << "exit";
}


// setup tranform for coordinates x: 0 - 100; y: 0 - 100; 
//  (0,0) is the bottom left corner
QTransform *PolarWidget::configureTextTransform()
{
  LOG(DEBUG) << "enter";
  
  // set world view

  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  int colorScaleWidth = _params->color_scale_width;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;


  return computeTextTransform(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   // xMinWorld, yMinWorld
                   0, 0,
                   // xMaxWorld, yMaxWorld
                   100, 100,
                   axisTickLen, nTicksIdeal, textMargin);

  LOG(DEBUG) << "exit";

}

QTransform *PolarWidget::computeTextTransform(
                    int widthPixels,
                    int heightPixels,
                    int leftMargin,
                    int rightMargin,
                    int topMargin,
                    int bottomMargin,
                    int colorScaleWidth,
                    double xMinWorld,
                    double yMinWorld,
                    double xMaxWorld,
                    double yMaxWorld,
                    int axisTickLen,
                    int nTicksIdeal,
                    int textMargin) {

  QTransform *transform = new QTransform();
  int plotWidth = widthPixels - leftMargin - rightMargin - colorScaleWidth;
  int plotHeight = heightPixels - topMargin - bottomMargin;
    
  int xMinPixel = leftMargin;
  int xMaxPixel = xMinPixel + plotWidth - 1;
  // OR ...
  //_yMinPixel = _topMargin + _plotHeight - 1;
  //_yMaxPixel = _topMargin;

  int yMinPixel = topMargin; 
  int yMaxPixel = yMinPixel + plotHeight - 1;
 
    
  double xPixelsPerWorld =
    abs((xMaxPixel - xMinPixel) / (xMaxWorld - xMinWorld));
  double yPixelsPerWorld =
    abs((yMaxPixel - yMinPixel) / (yMaxWorld - yMinWorld));
    
  transform->reset();
  transform->scale(xPixelsPerWorld, yPixelsPerWorld);

  return transform;
}



/*************************************************************************
 * configure the axes
 */

/*************************************************************************
 * set archive mode
 */

//void PolarWidget::setArchiveMode(bool archive_mode)
//{
//  _archiveMode = archive_mode;
//}

/*************************************************************************
 * unzoom the view
 */

void PolarWidget::unzoomView()
{
  LOG(DEBUG) << "enter";
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();
  update();
  LOG(DEBUG) << "exit";
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



// TODO: this should be a slot?
// TODO: sort out new fields in PolarManager
// Who needs to know about the new fields?
// Beams, FieldRenderers, DisplayFieldController 
void PolarWidget::addNewFields(vector<DisplayField *> &newFields) 
{
  LOG(DEBUG) << "enter";

  LOG(DEBUG) << "exit";
}

/*************************************************************************
 * turn on archive-style rendering - all fields
 */

void PolarWidget::activateArchiveRendering()
{
  LOG(DEBUG) << "enter";

  LOG(DEBUG) << "exit";
}


/*************************************************************************
 * displayImage()
 */

void PolarWidget::displayImage(string currentFieldName, double currentSweepAngle,
  ColorMap &colorMap,
  QColor backgroundColor)
{
  try {

    // set the context ...
    _currentSweepAngle = currentSweepAngle;
    //_rayLocationController = rayLocationController;
    _currentColorMap = colorMap;
    _backgroundColor = backgroundColor;
    
    update();
  } catch (std::range_error &ex) {
    LOG(ERROR) << ex.what();
    //QMessageBox::warning(NULL, "Error changing field (displayImage):", ex.what());
  }
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
}


/*************************************************************************
 * gridRingsColor()
 */

void PolarWidget::gridRingsColor(const QColor &color)
{
  LOG(DEBUG) << "enter " << color.name().toStdString();
  _gridRingsColor = color;
  update();
  LOG(DEBUG) << "exit";
}

/*************************************************************************
 * ColorScaleLegend()
 */

void PolarWidget::colorScaleLegend()
{
  LOG(DEBUG) << "enter " ;
  update();
  LOG(DEBUG) << "exit";
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

void PolarWidget::mapPixelToWorld(int x, int y, double *worldX, double *worldY) {
        qreal sx = _zoomTransform.m11();
        qreal sy = _zoomTransform.m22();
        qreal dx = _zoomTransform.dx();
        qreal dy = _zoomTransform.dy();        
        *worldY = (y - dy) / sy;
        *worldX = (x - dx) / sx;
}

/*************************************************************************
 * Slots
 *************************************************************************/

/*************************************************************************
 * mousePressEvent()
 */

void PolarWidget::mousePressEvent(QMouseEvent *e)
{

  _mousePressX = e->x();
  _mousePressY = e->y();

  mapPixelToWorld(_mousePressX, _mousePressY, &_worldPressX, &_worldPressY);

  if (_manager->isOverBoundaryPoint(_worldPressX, _worldPressY)) {
      _rubberBand->hide();
      _boundaryTrackMouseMove = true;
  } else {
      _rubberBand->setGeometry(QRect(e->pos(), QSize()));
      _rubberBand->show();
  }
}


/*************************************************************************
 * mouseMoveEvent(), mouse button is down and mouse is moving
 */

void PolarWidget::mouseMoveEvent(QMouseEvent * e)
{
  double worldX;
  double worldY;

  mapPixelToWorld(e->pos().x(), e->pos().y(), &worldX, &worldY);
  
  if (_boundaryTrackMouseMove) {
    _manager->moveBoundaryPoint(_worldPressX, _worldPressY,
      worldX, worldY);
    update();
    return;    
  }
   
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
  QPointF clickPos(e->pos());
  
  _mouseReleaseX = clickPos.x();
  _mouseReleaseY = clickPos.y();

  mapPixelToWorld(_mouseReleaseX, _mouseReleaseY, &_worldReleaseX, &_worldReleaseY);
  // get click location in world coords

  // --- if shift key is down, then pass message on to boundary point control
  bool isShiftKeyDown = (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
  if (isShiftKeyDown) {
    _manager->addDeleteBoundaryPoint(_worldReleaseX, _worldReleaseY, isShiftKeyDown);
  } else {
    if (rgeom.width() <= 20) {
      // If the mouse hasn't moved much, assume we are clicking rather than
      // zooming
  
      double x_km = _worldReleaseX;
      double y_km = _worldReleaseY;
      _pointClicked = true;
      // get ray closest to click point
      const RadxRay *closestRay = _getClosestRay(x_km, y_km);
      // Emit a signal to indicate that the click location has changed
      emit locationClicked(x_km, y_km, closestRay);
    } else {
      // mouse moved more than 20 pixels, so a zoom occurred
      // or moving boundary points

      // if mouse was pressed near a boundary point, then move the boundary point
      // otherwise, this is a zoom
      if (_boundaryTrackMouseMove) {
        _manager->moveBoundaryPoint(_worldPressX, _worldPressY,
          _worldReleaseX, _worldReleaseY);
      } else {

        // use rgeom

        int xMin, xMax, yMin, yMax;
        rgeom.getCoords(&xMin, &yMax, &xMax, &yMin);

        double xMinWorld, yMinWorld, xMaxWorld, yMaxWorld;

        mapPixelToWorld(xMax, yMax, &xMaxWorld, &yMaxWorld);
        mapPixelToWorld(xMin, yMin, &xMinWorld, &yMinWorld);

        _zoomWorld.set2(xMinWorld, yMinWorld, xMaxWorld, yMaxWorld);
        _setTransform(_zoomWorld.getTransform());

        _setGridSpacing();
        _manager->enableZoomButton();        
      }
    }
  }  
  _rubberBand->hide();
  _boundaryTrackMouseMove = false;
  update();
  
}

//void PolarWidget::imageReady(QImage *image) {
//  _image = image;  // TODO: make sure this isn't a copy!  just assign a pointer
//  update();
//}

/*************************************************************************
 * paintEvent()
 */

void PolarWidget::paintEvent(QPaintEvent *event)
{
  static int trial= 0;
  LOG(DEBUG) << "enter";

  QImage m_image;
  QImage m_buffer;
  QImage m_base_buffer;

  try {
    QPainter painter(this); //
    // QPainter painter(this);  If you paint to a Widget, can only be done inside paintEvent

    string selectedField = displayFieldController->getSelectedFieldName();
    if (selectedField.length() > 0) {

        // using a QImage
        if (_rayLocationController == NULL) {
          cerr << "something crazy happened: bailing!" << endl;
          return;
        } else { // m_buffer.size() != size()) {
            LOG(DEBUG) << " inside first QImage size = " << width() << " x " << height();
            m_buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
            m_base_buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);

            m_base_buffer.fill(_backgroundBrush.color());

            QPainter p(&m_base_buffer);
            p.setTransform(_zoomTransform);
            //drawBase(p);
            // draw into the QImage
  
            _fieldRendererController->renderImage(p, 0, 0, // 0 means they aren't used; width(), height(), 
              selectedField, _zoomTransform, 
              _currentSweepAngle,
              _rayLocationController, _currentColorMap, _backgroundBrush.color());
        }

        // TODO: move to sizeInBytes when no longer supporting Centos7;
        // byteCount() and numBytes() are obsolete.
        //memcpy(m_buffer.bits(), m_base_buffer.bits(), m_buffer.sizeInBytes());
        memcpy(m_buffer.bits(), m_base_buffer.bits(), m_buffer.byteCount());

        {
            QPainter p(&m_buffer);
            p.setTransform(_zoomTransform);
            _drawOverlays(p);
            _manager->drawBoundary(_zoomWorld, p);  
            //if there are no points, this does nothing
            // todo overlay boundary image
        }

        bool horizontal = false;
        bool vertical = true;
        painter.drawImage(0, 0, m_buffer);
      // need to plot data in a QImage, so that the overlays don't wipe out the data
   
    }
  } catch (const std::out_of_range& ex) {
    LOG(DEBUG) << ex.what();
  } catch (std::range_error &ex) {
      LOG(ERROR) << ex.what();
      //QMessageBox::warning(NULL, "Error changing field (_changeField):", ex.what());
  } catch (std::invalid_argument &ex) {
    _manager->errorMessage("Error", ex.what());
  }
  LOG(DEBUG) << "exit";
}

void PolarWidget::showSelectedField()
{
  LOG(DEBUG) << "enter";
  try {
    string selectedField = displayFieldController->getSelectedFieldName();
    if (selectedField.length() > 0) {

    } else {
      LOG(DEBUG) << "selected field is empty";
    }

  } catch (const std::out_of_range& ex) {
    LOG(DEBUG) << ex.what();
  } catch (std::range_error &ex) {
      LOG(ERROR) << ex.what();
      //QMessageBox::warning(NULL, "Error changing field (_changeField):", ex.what());
  }
  LOG(DEBUG) << "exit";
}


/*************************************************************************
 * resizeEvent()
 */

void PolarWidget::resizeEvent(QResizeEvent * e)
{ 
  LOG(DEBUG) << "enter";
  _resetWorld(width(), height());
  LOG(DEBUG) << "exit";
}

/*************************************************************************
 * resize()
 */

void PolarWidget::resize(const int width, const int height)
{

  LOG(DEBUG) << "enter";
  // Set the geometry based on the aspect ratio that we need for this display.
  // The setGeometry() method will fire off the resizeEvent() so we leave the
  // updating of the display to that event.
  
  int sizeNeeded = (int) ((width - _colorScaleWidth) / _aspectRatio + 0.5);
  if (height < sizeNeeded) {
    sizeNeeded = height;
  }
  // setGeometry triggers a resizeEvent -- No, it does not trigger the event
  int new_width = (int) (sizeNeeded * _aspectRatio + 0.5) + _colorScaleWidth;
  int new_height = sizeNeeded;
  setGeometry(0, 0, new_width, new_height);

  LOG(DEBUG) << "exit";
}

//////////////////////////////////////////////////////////////
// reset the pixel size of the world view

void PolarWidget::_resetWorld(int width, int height)

{
  LOG(DEBUG) << "enter";
  _fullWorld.resize(width, height);
  _zoomWorld = _fullWorld;
  _setTransform(_fullWorld.getTransform());
  _setGridSpacing();
  _dirty = true;
  LOG(DEBUG) << "exit";
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
  
void PolarWidget::informationMessage()
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

void PolarWidget::notImplemented()
{
  cerr << "inside notImplemented() ... " << endl;

  QErrorMessage *errorMessageDialog = new QErrorMessage(_parent);
  // QLabel *informationLabel = new QLabel();

  errorMessageDialog->showMessage("This option is not implemented yet.");
  QLabel errorLabel;
  int frameStyle = QFrame::Sunken | QFrame::Panel;
  errorLabel.setFrameStyle(frameStyle);
  errorLabel.setText("If the box is unchecked, the message "
		     "won't appear again.");

  cerr << "exiting notImplemented() " << endl;

}



////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *PolarWidget::_getClosestRay(double x_km, double y_km)

{
  return NULL;
}

/*************************************************************************
 * _setGridSpacing()
 */

void PolarWidget::_setGridSpacing()
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


void PolarWidget::_drawOverlays(QPainter &painter)
{

  LOG(DEBUG) << "enter";
      //painter.drawText(10, 10, "theText");
  drawRings(painter);
  drawGrid(painter);
  drawAzimuthLines(painter);
  drawColorScale(painter);
  drawLegend(painter);
  LOG(DEBUG) << "exit";

}

void PolarWidget::drawRings(QPainter &painter)
{

  LOG(DEBUG) << "enter";

  // Don't try to draw rings if we haven't been configured yet or if the
  // rings or grids aren't enabled.
  
  if (!_ringsEnabled && !_gridsEnabled && !_angleLinesEnabled) {
    return;
  }

  // save painter state

  painter.save();

  // store font
  
  QFont origFont = painter.font();

  QPen pen(_gridRingsColor);
  qreal hscale = painter.combinedTransform().m11();
  qreal vscale = painter.combinedTransform().m22();
  qreal htranslate = painter.combinedTransform().m31();
  qreal vtranslate = painter.combinedTransform().m32();

  float width = 1.0/hscale * 2.0;
  if (width <= 0) width = 1.0;
  pen.setWidth(width);

  painter.setPen(pen);
  
  // Draw rings

  if (_ringSpacing > 0.0 && _ringsEnabled) {
  
    LOG(DEBUG) << "_ringSpacing = " << _ringSpacing;
    double ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      QRectF rect(-ringRange, -ringRange, ringRange * 2.0, ringRange * 2.0);
      painter.drawEllipse(rect);
      ringRange += _ringSpacing;
    }

    // Draw the labels
    painter.save();

    QTransform xform = painter.worldTransform();

    QTransform identity;
    painter.setWorldTransform(identity, false);
    QFont font = painter.font();

    font.setPointSize(12);
    painter.setFont(font);

    ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      double labelPosD = ringRange * SIN_45;
      const string &labelStrS = _scaledLabel.scale(ringRange);
      int labelPos = (int) labelPosD;
      QString labelStr(labelStrS.c_str());

      qreal x; 
      qreal y;
      qreal labelPosQR = (qreal) labelPosD; 
      xform.map(labelPosQR, labelPosQR, &x, &y);
      painter.drawText(  x, y, labelStr);

      xform.map(-labelPosQR, labelPosQR, &x, &y);
      painter.drawText(  x, y, labelStr);

      xform.map(labelPosQR, -labelPosQR, &x, &y);
      painter.drawText(  x, y, labelStr);

      xform.map(-labelPosQR, -labelPosQR, &x, &y);
      painter.drawText(  x, y, labelStr);

      ringRange += _ringSpacing;
    }
    
    painter.restore();
    
  } /* endif - draw rings */

  painter.restore();

  LOG(DEBUG) << "exit";

}  

void PolarWidget::drawGrid(QPainter &painter) {
  
  // Draw the grid

  if (_ringSpacing > 0.0 && _gridsEnabled)  {

    // Set up the painter
    
    painter.save();
    //painter.setTransform(_zoomTransform);
    painter.setPen(_gridRingsColor);
  
    double ringRange = _ringSpacing;
    double maxRingRange = ringRange;
    while (ringRange <= _maxRangeKm) {

      painter.drawLine( ringRange, -_maxRangeKm, ringRange, _maxRangeKm);
      painter.drawLine( -ringRange, -_maxRangeKm, -ringRange, _maxRangeKm);
      painter.drawLine( -_maxRangeKm, ringRange, _maxRangeKm, ringRange);
      painter.drawLine( -_maxRangeKm, -ringRange, _maxRangeKm, -ringRange);
      
      maxRingRange = ringRange;
      ringRange += _ringSpacing;
    }
    painter.restore();

    _zoomWorld.setSpecifyTicks(true, -maxRingRange, _ringSpacing);

    _zoomWorld.drawAxisLeft(painter, "km", true, true, true);
    _zoomWorld.drawAxisRight(painter, "km", true, true, true);
    _zoomWorld.drawAxisTop(painter, "km", true, true, true);
    _zoomWorld.drawAxisBottom(painter, "km", true, true, true);
    
    _zoomWorld.setSpecifyTicks(false);

  }
  
  LOG(DEBUG) << "exit";

}  

void PolarWidget::drawAzimuthLines(QPainter &painter) { 
  // Draw the azimuth lines

  if (_angleLinesEnabled) {

    // Set up the painter
    
    painter.save();
    QPen pen(_gridRingsColor);
    qreal hscale = painter.combinedTransform().m11();
    pen.setWidth(1.0/hscale * 2.0);
    painter.setPen(pen);


  
    // Draw the lines along the X and Y axes

    painter.drawLine(0, -_maxRangeKm, 0, _maxRangeKm);
    painter.drawLine(-_maxRangeKm, 0, _maxRangeKm, 0);

    // Draw the lines along the 30 degree lines

    double end_pos1 = SIN_30 * _maxRangeKm;
    double end_pos2 = COS_30 * _maxRangeKm;
    
    painter.drawLine( end_pos1, end_pos2, -end_pos1, -end_pos2);
    painter.drawLine( end_pos2, end_pos1, -end_pos2, -end_pos1);
    painter.drawLine( -end_pos1, end_pos2, end_pos1, -end_pos2);
    painter.drawLine( end_pos2, -end_pos1, -end_pos2, end_pos1);

    painter.restore();

  }
  
  // click point cross hairs
  
  if (_pointClicked) {

    int startX = _mouseReleaseX - _params->click_cross_size / 2;
    int endX = _mouseReleaseX + _params->click_cross_size / 2;
    int startY = _mouseReleaseY - _params->click_cross_size / 2;
    int endY = _mouseReleaseY + _params->click_cross_size / 2;

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

  }

  // reset painter state
  
  //painter.restore();

  LOG(DEBUG) << "exit";

}  


/////////////////////////////////////////////////////
// draw the color scale

void PolarWidget::drawColorScaleFromWorldPlot(const ColorMap &colorMap,
                               QPainter &painter,
                               int unitsFontSize)
  
{
  LOG(DEBUG) << "enter";

  const std::vector<ColorMap::CmapEntry> &cmap = colorMap.getEntries();

  // plot using x:0 - 100, y: 0 - 100; (0,0) upper left corner
  QTransform *textTransform = configureTextTransform();
  painter.setWorldTransform(*textTransform); 

  //int _plotHeight = height();
  int _widthPixels = width();


  int pltHt = 100; // _plotHeight;
  int width = _colorScaleWidth;
  //double widthInPixels = width();
  double widthInWorld = 100;

  double colorScaleWidthScaled = _colorScaleWidth * widthInWorld / _widthPixels; // widthInPixels;;
  int xStart = 100; // _widthPixels - width;
  size_t nHts = cmap.size() + 1; // leave some space at top and bottom
  double patchHt = (double)(pltHt) / nHts;
  int iPatchHt = (int) patchHt;
  int bottomMargin = (int) patchHt/2.0;

  // fill the swatches with the color
  
  //painter.save();
  painter.setPen(Qt::SolidLine);
  int scaleYTop = 0, scaleYBot = 0;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    QColor color(entry.red, entry.green, entry.blue);
    painter.setBrush(color);
    //double topY = pltHt - (int) (ii + 2) * patchHt + (patchHt / 2) + _topMargin;
    double topY = (100-bottomMargin) - (ii+1)*patchHt;
    QRectF r(xStart, topY, width, patchHt);
    painter.fillRect(r, color);
    if (ii == 0) {
      scaleYBot = topY + patchHt;
    } else if (ii == cmap.size() - 1) {
      scaleYTop = topY;
    }
  }
  //painter.restore();

  // get precision based on data
  
  double minDelta = 1.0e99;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    double delta = fabs(entry.maxVal - entry.minVal);
    if (delta < minDelta) minDelta = delta;
  }
  int ndecimals = 0;
  char format = 'f';
  if (minDelta <= 0.025) {
    ndecimals = 3;
    format = 'g';
  } else if (minDelta <= 0.05) {
    ndecimals = 3;
  } else if (minDelta <= 0.25) {
    ndecimals = 2;
  } else if (minDelta <= 25) {
    ndecimals = 1;
  }

  // save state

  //painter.save();

  // scale the font
  
  QFont font = painter.font();

  font.setPointSize(2);
  painter.setFont(font);
  
  // add labels

  painter.setBrush(Qt::black);
  painter.setBackgroundMode(Qt::OpaqueMode);
  QRect tRect(painter.fontMetrics().tightBoundingRect("1.0"));
  int textHt = tRect.height();

  int xStartPlusABit = xStart + colorScaleWidthScaled/4;  
  
  if (colorMap.labelsSetByValue()) {

    LOG(DEBUG) << "colorMap.labelsSetByValue";
  
    // label values specified in the color scale file

    const vector<ColorMap::CmapLabel> &labels = colorMap.getSpecifiedLabels();
    double scaleHeight = scaleYBot - scaleYTop;
    for (size_t ii = 0; ii < labels.size(); ii++) {
      const ColorMap::CmapLabel &label = labels[ii];
      double yy = scaleYBot - scaleHeight * label.position;
      double yPos = 100 - bottomMargin - ii*patchHt - (textHt+4)/2;
      painter.drawText(xStartPlusABit, 
                       (int) yPos, // yy - textHt / 2, 
                       width + 4, textHt + 4, 
                       Qt::AlignLeft | Qt::AlignVCenter, // Qt::AlignCenter | Qt::AlignHCenter, 
                       label.text.c_str());
    } // ii

  } else {
    LOG(DEBUG) << "label the color transitions";

    // label the color transitions
    // we space the labels vertically by at least 2 * text height
    
    //double yy = pltHt - (patchHt * 1.0) + _topMargin;
    double prevIyy = -1;

    for (size_t ii = 0; ii < cmap.size(); ii++) {
      const ColorMap::CmapEntry &entry = cmap[ii];
      QString label = QString("%1").arg(entry.minVal,0,format,ndecimals);

        double yPos = 100 - bottomMargin - ii*patchHt - (textHt+4)/2; // /2;
        painter.drawText(xStartPlusABit, //  xStart+5, 
                         (int) yPos, // iyy, 
                         //width, iPatchHt, 
                         width + 4, textHt + 4, 
                         Qt::AlignLeft | Qt::AlignVCenter, // Qt::AlignCenter | Qt::AlignHCenter, 
                         label);
    }
    
    // last label at top
    
    const ColorMap::CmapEntry &entry = cmap[cmap.size()-1];
    QString label = QString("%1").arg(entry.maxVal,0,format,ndecimals);
    int ii = cmap.size();
    double yPos = 100 - bottomMargin - ii*patchHt - (textHt+4)/2; // /2;
    painter.drawText(xStartPlusABit,
                         (int) yPos, // iyy, 
                         //width, iPatchHt, 
                         width + 4, textHt + 4, 
                         Qt::AlignLeft | Qt::AlignVCenter, // Qt::AlignCenter | Qt::AlignHCenter, 
                         label);    
  }

/*  // add Units label

  string units(colorMap.getUnits());
  if (units.size() > 0) {
    
    QFont ufont(painter.font());
    ufont.setPointSizeF(unitsFontSize);
    painter.setFont(ufont);

    QRect tRect(painter.fontMetrics().tightBoundingRect(units.c_str()));
    int iyy = _topMargin / 2;
    int ixx = _widthPixels - width;
    QString qunits(units.c_str());
    painter.drawText(ixx, iyy, width, tRect.height() + 4, 
                     Qt::AlignTop | Qt::AlignHCenter, qunits);

  }
*/
  // restore state

  // painter.restore();

  LOG(DEBUG) << "exit";

}




void PolarWidget::drawColorScale(QPainter &painter) {
  // draw the color scale
  LOG(DEBUG) << "enter";

    painter.save();
    //painter.setPen(Qt::yellow);
    //painter.setBrush(Qt::black);
    //painter.setBackgroundMode(Qt::OpaqueMode);

    //painter.setBrush(Qt::white);
    painter.setBackgroundMode(Qt::TransparentMode);


    // plot using x:0 - 100, y: 0 - 100; (0,0) bottom left corner
    QTransform *textTransform = configureTextTransform();
    painter.setWorldTransform(*textTransform); 

    //QFont font = painter.font();
    //font.setPointSize(3);
    //painter.setFont(font);  

  DisplayField *field = displayFieldController->getSelectedField();
  drawColorScaleFromWorldPlot(field->getColorMap(), painter,
                            _params->label_font_size);

  painter.restore();
  delete textTransform;

  LOG(DEBUG) << "exit";

}

void PolarWidget::drawLegend(QPainter &painter) {

    // add legends with time, field name and elevation angle

    vector<string> legends;
    char text[1024];

    // time legend

    //sprintf(text, "Start time: %s", _plotStartTime.asString(0).c_str());
    //legends.push_back(text);
    
    // radar and site name legend

    string radarName(_platform.getInstrumentName());
    if (_params->override_radar_name) {
      radarName = _params->radar_name;
    }
    string siteName(_platform.getInstrumentName());
    if (_params->override_site_name) {
      siteName = _params->site_name;
    }
    string radarSiteLabel = radarName;
    if (siteName.size() > 0 && siteName != radarName) {
      radarSiteLabel += "/";
      radarSiteLabel += siteName;
    }
    legends.push_back(radarSiteLabel);

    // field name legend
    //size_t selectedField = displayFieldController->getSelectedFieldNum();
 
    //if (0) {
    //FieldRenderer *selectedFieldRenderer = _fieldRendererController->get(selectedField);
    //string fieldName = selectedFieldRenderer->getField().getLabel();
    //}
    string fieldName = displayFieldController->getSelectedFieldName();


    //string fieldName = _fieldRenderers[_selectedField]->getField().getLabel();
    sprintf(text, "Field: %s", fieldName.c_str());
    legends.push_back(text);

    // elevation legend

    //sprintf(text, "Elevation(deg): %.2f", _meanElev);
    //legends.push_back(text);

    // nrays legend

    //sprintf(text, "NRays: %g", _nRays);
    //legends.push_back(text);


  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  int colorScaleWidth = _params->color_scale_width;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;    
    
    painter.save();
    painter.setPen(Qt::yellow);
    //painter.setBrush(Qt::black);
    //painter.setBackgroundMode(Qt::OpaqueMode);

    //painter.setBrush(Qt::white);
    painter.setBackgroundMode(Qt::TransparentMode);


    // plot using x:0 - 100, y: 0 - 100; (0,0) bottom left corner
    QTransform *textTransform = configureTextTransform();
    painter.setWorldTransform(*textTransform); 

    QFont font = painter.font();
    font.setPointSize(3);
    painter.setFont(font);    

    qreal xx, yy;

    //switch (_params->ppi_main_legend_pos) {
      //case Params::LEGEND_TOP_LEFT:
        xx = 0;  // (qreal) (_xMinPixel + _axisTickLen + _textMargin);
        yy = 0; //_yMaxPixel + _axisTickLen;
        //_zoomWorld.drawLegendsTopLeft(painter, legends);
        /*
        break;
      case Params::LEGEND_TOP_RIGHT:
        //_zoomWorld.drawLegendsTopRight(painter, legends);
        qreal yy = _yMaxPixel + _axisTickLen;
        break;
      case Params::LEGEND_BOTTOM_LEFT:
        //_zoomWorld.drawLegendsBottomLeft(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_RIGHT:
        //_zoomWorld.drawLegendsBottomRight(painter, legends);
        break;
      default: {}
    }
    */

  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy += (textMargin + tRect.height());
  }

/* for top right ...
  for (size_t i = 0; i < legends.size(); i++) {
    string legend(legends[i]);
    QRect tRect(painter.fontMetrics().tightBoundingRect(legend.c_str()));
    qreal xx = (qreal) (_xMaxPixel - _axisTickLen -
                        _textMargin - tRect.width());
    QRectF bRect(xx, yy, tRect.width() + 2, tRect.height() + 2);
    painter.drawText(bRect, Qt::AlignCenter, legend.c_str());
    yy += (_textMargin + tRect.height());
  }   
  */ 


  painter.restore();
  delete textTransform;

  LOG(DEBUG) << "exit";

}

//void PolarWidget::drawColorScaleLegend(QPainter &painter) {

/*
  // draw the color scale

  DisplayField *field = displayFieldController->getSelectedField();
  drawColorScale(field->getColorMap(), painter,
                            _params->label_font_size);
*/
//}

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

void PolarWidget::_drawScreenText(QPainter &painter, 
  const string &text,
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
