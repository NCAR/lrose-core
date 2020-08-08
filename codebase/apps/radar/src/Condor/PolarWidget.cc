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

  _isArchiveMode = false; // ??
  _archiveMode = _params.begin_in_archive_mode;

  _titleMargin = _params.main_window_title_margin;
  _aspectRatio = _params.plot_aspect_ratio;
  _colorScaleWidth = _params.color_scale_width;
  _fullWorld.setColorScaleWidth(_colorScaleWidth);
  _fullWorld.setTitleTextMargin(_titleMargin);
  _fullWorld.setTopMargin(_titleMargin);

  _nRows = _params.plots_n_rows;
  _nCols = _params.plots_n_columns;
  _nPlots = _nRows * _nCols;
  
  _plotsGrossHeight = height() - _titleMargin - 1;
  _plotsGrossWidth = width() - _colorScaleWidth - 1;
  _plotWidth = _plotsGrossWidth / _nCols;
  _plotHeight = _plotsGrossHeight / _nRows;

  _ppiPlotsConfigured = false;
  _rhiPlotsConfigured = false;

  _xGridEnabled = false;
  _yGridEnabled = false;

  _currentBeam = NULL;

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

  // qRegisterMetaType<size_t>("size_t");

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
  _pointClicked = false;
  _mousePressX = 0;
  _mousePressY = 0;
  _mouseReleaseX = 0;
  _mouseReleaseY = 0;
  _zoomCornerX = 0;
  _zoomCornerY = 0;
  
  _openingFileInfoLabel = new QLabel("Opening file, please wait...", parent);
  _openingFileInfoLabel->setStyleSheet("QLabel { background-color : darkBlue; color : yellow; qproperty-alignment: AlignCenter; }");
  _openingFileInfoLabel->setVisible(false);

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
  LOG(DEBUG) << "enter " << color.name().toStdString();
  _gridRingsColor = color;
  update();
  LOG(DEBUG) << "exit";
}


/*************************************************************************
 * getImage()
 */

QImage* PolarWidget::getImage()
{
  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;
}


/*************************************************************************
 * getPixmap()
 */

QPixmap* PolarWidget::getPixmap()
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

void PolarWidget::mousePressEvent(QMouseEvent *e)
{
  
  if (e->button() == Qt::RightButton) {
    QPointF clickPos(e->pos());
    _mousePressX = e->x();
    _mousePressY = e->y();
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);
    emit customContextMenuRequested(clickPos.toPoint());
  } else {
    _rubberBand->setGeometry(QRect(e->pos(), QSize()));
    _rubberBand->show();
    _mousePressX = e->x();
    _mousePressY = e->y();
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);
  }

}


/*************************************************************************
 * mouseMoveEvent(), mouse button is down and mouse is moving
 */

void PolarWidget::mouseMoveEvent(QMouseEvent * e)
{

  int worldX = (int)_zoomWorld.getXWorld(e->pos().x());
  int worldY = (int)_zoomWorld.getYWorld(e->pos().y());
  
  if (_manager._boundaryEditorDialog->isVisible()) {

    BoundaryToolType tool = BoundaryPointEditor::Instance()->getCurrentTool();
    
    if (tool == BoundaryToolType::polygon && 
        BoundaryPointEditor::Instance()->isAClosedPolygon() && 
        BoundaryPointEditor::Instance()->isOverAnyPoint(worldX, worldY)) {
      BoundaryPointEditor::Instance()->moveNearestPointTo(worldX, worldY);
    } else if (tool == BoundaryToolType::brush) {
      BoundaryPointEditor::Instance()->addToBrushShape(worldX, worldY);
    }

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
  
  if (e->button() == Qt::RightButton) {
    
    QPointF clickPos(e->pos());
    
    _mousePressX = e->x();
    _mousePressY = e->y();
    
    emit customContextMenuRequested(clickPos.toPoint()); // , closestRay);
    
  } else {
    
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

// Used to notify BoundaryPointEditor if the user has zoomed in/out or is pressing the Shift key
// Todo: investigate implementing a listener pattern instead
void PolarWidget::timerEvent(QTimerEvent *event)
{
  bool doUpdate = false;
  bool isBoundaryEditorVisible = _manager._boundaryEditorDialog->isVisible();
  if (isBoundaryEditorVisible) {
    double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
    doUpdate = BoundaryPointEditor::Instance()->updateScale(xRange);   //user may have zoomed in or out, so update the polygon point boxes so they are the right size on screen
  }
  bool isBoundaryFinished = BoundaryPointEditor::Instance()->isAClosedPolygon();
  bool isShiftKeyDown =
    (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
  if ((isBoundaryEditorVisible && !isBoundaryFinished) ||
      (isBoundaryEditorVisible && isBoundaryFinished && isShiftKeyDown)) {
    this->setCursor(Qt::CrossCursor);
  } else {
    this->setCursor(Qt::ArrowCursor);
  }
  
  if (doUpdate) {  //only update if something has changed
    cerr << "UUUUUUUUUUUUUUUUUU222222222222222" << endl;
    update();
  }

}


/**************   testing ******/
void PolarWidget::smartBrush(int xPixel, int yPixel) {
  //int xp = _ppi->_zoomWorld.getIxPixel(xkm);
  //int yp = _ppi->_zoomWorld.get>IyPixel(ykm);

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

  // compute plot widths

  _plotsGrossWidth = width() - 1 - _colorScaleWidth;
  _plotsGrossHeight = height() - 1 - _titleMargin;
  _plotWidth = _plotsGrossWidth / _nCols;
  _plotHeight = _plotsGrossHeight / _nRows;
  
  cerr << "22222222222222222222" << endl;

  QPainter painter(this);

  // painter.drawImage(0, 0, *(_fieldRenderers[_selectedField]->getImage()));

  // _drawOverlays(painter);
  _drawDividers(painter);

  // draw the color scale

  const DisplayField &field = _manager.getSelectedField();
  _fullWorld.drawColorScale(field.getColorMap(), painter,
                            _params.label_font_size);

  // BoundaryPointEditor::Instance()->draw(_zoomWorld, painter);  //if there are no points, this does nothing

  cerr << "aaaaaaaaaaaaaaaaaaaaa" << endl;

}


/*************************************************************************
 * resizeEvent()
 */

void PolarWidget::resizeEvent(QResizeEvent * e)
{
  cerr << "333333333333333333" << endl;
  _resetWorld(width(), height());
  _refreshImages();
  cerr << "UUUUUUUUUUUUUUUUUU33333333333333" << endl;
  // update();
}


/*************************************************************************
 * overload resize()
 */

void PolarWidget::resize(int ww, int hh)
{
  
  double grossHeight = hh - _titleMargin - 1;
  double grossWidth = ww - _colorScaleWidth - 1;
  double grossAspect = grossWidth / grossHeight;
  double plotWidth = grossWidth / _nCols;
  double plotHeight = grossHeight / _nRows;

  if (_params.plot_aspect_ratio < 0) {

    // use aspect ratio from window
    _aspectRatio = grossAspect;

  } else {

    // use specified aspect ratio

    _aspectRatio = _params.plot_aspect_ratio;
    cerr << "AAAAAAAAAAA grossAspect, _aspectRatio: " << grossAspect << ", " << _aspectRatio << endl;
    if (_aspectRatio > grossAspect) {
      // limit height
      plotHeight = plotWidth / _aspectRatio;
    } else {
      // limit width
      plotWidth = plotHeight * _aspectRatio;
    }
    
  }
  
  _plotWidth = (int) (plotWidth + 0.5);
  _plotHeight = (int) (plotHeight + 0.5);
  _plotsGrossWidth = _nCols * _plotWidth;
  _plotsGrossHeight = _nRows * _plotHeight;
  int totalWidth = _plotsGrossWidth + _colorScaleWidth + 1;
  int totalHeight = _plotsGrossHeight + _titleMargin + 1;

  // QWidget::resize(totalWidth, totalHeight);

  setGeometry(0, 0,  totalWidth, totalHeight);

  cerr << "RRRRRRRRRRRRRRR plotWidth, plotHeight: " << _plotWidth << ", " << _plotHeight << endl;
  cerr << "RRRRRRRRRRRRRRR plotGrossWidth, plotGrossHeight: " << _plotsGrossWidth << ", " << _plotsGrossHeight << endl;
  cerr << "RRRRRRRRRRRRRRR titleMargin, colorScaleWidth: " << _titleMargin << ", " << _colorScaleWidth << endl;
  cerr << "RRRRRRRRRRRRRRRR resize ww, hh: " << ww << ", " << hh << endl;
  cerr << "RRRRRRRRRRRRRRRR resize width, height: " << this->width() << ", " << this->height() << endl;
  cerr << "RRRRRRRRR _aspectRatio: " << _aspectRatio << endl;

  // Set the geometry based on the aspect ratio that we need for this display.
  // The setGeometry() method will fire off the resizeEvent() so we leave the
  // updating of the display to that event.
  
  // int sizeNeeded = (int) ((ww - _colorScaleWidth) / _aspectRatio + 0.5);
  // if (hh < sizeNeeded) {
  //   sizeNeeded = hh;
  // }

  // int widthNeeded = (int) (sizeNeeded * _aspectRatio + 0.5) + _colorScaleWidth;

  // QWidget::resize(700, 500);


  cerr << "UUUUUUUUUUUUUUUUUU111111111111" << endl;
  update();

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
	float worldScale = _zoomWorld.getXMaxWindow() - _zoomWorld.getXMinWindow();
 	BoundaryPointEditor::Instance()->setWorldScale(worldScale);

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

  cerr << "UUUUUUUUUUUUUUUUUU555555555555555" << endl;
  // update();

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


// slots for context editing; create and show the associated modeless dialog and return                                   

void PolarWidget::contextMenuCancel()
{
  //  informationMessage();

  //notImplemented();                                                                                                     
}

void PolarWidget::contextMenuParameterColors()
{
  /*
  LOG(DEBUG) << "enter";

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

  LOG(DEBUG) << "exit ";
  */
  informationMessage();
   
}

void PolarWidget::contextMenuView()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}

// void PolarWidget::contextMenuEditor()
// {
//   informationMessage();
//   //  notImplemented();                                                                                                   
// }


void PolarWidget::contextMenuExamine()         
{
  informationMessage();                                                                                                 

}

void PolarWidget::contextMenuDataWidget()
{
  informationMessage();

  //  notImplemented();                                                                                                   
}


// void PolarWidget::ExamineEdit(const RadxRay *closestRay) 
// {
//   notImplemented();
// }

// void PolarWidget::ShowContextMenu(const QPoint &pos, RadxVol *vol) 
// {  
//   notImplemented();
// }

void PolarWidget::showOpeningFileMsg(bool isVisible)
{
  _openingFileInfoLabel->setGeometry(width()/2 - 120, height()/2 -15, 200, 30);
  _openingFileInfoLabel->setVisible(isVisible);
  _parent->update();
}

/*************************************************************************
 * configureRange()
 */

void PolarWidget::configureRange(double max_range)
{

  // Save the specified values

  _maxRangeKm = max_range;

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setGridSpacing();
  
  // set world view

  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  int colorScaleWidth = _params.color_scale_width;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;

  if (_params.ppi_display_type == Params::PPI_AIRBORNE) {

    _fullWorld.setWindowGeom(width(), height(), 0, 0);
    _fullWorld.setLeftMargin(leftMargin);
    _fullWorld.setRightMargin(rightMargin);
    _fullWorld.setTopMargin(topMargin);
    _fullWorld.setBottomMargin(bottomMargin);
    _fullWorld.setColorScaleWidth(colorScaleWidth);
    _fullWorld.setWorldLimits(-_maxRangeKm, 0.0, _maxRangeKm, _maxRangeKm);
    _fullWorld.setXAxisTickLen(axisTickLen);
    _fullWorld.setXNTicksIdeal(nTicksIdeal);
    _fullWorld.setAxisTextMargin(textMargin);

  } else {
    
    _fullWorld.setWindowGeom(width(), height(), 0, 0);
    _fullWorld.setLeftMargin(leftMargin);
    _fullWorld.setRightMargin(rightMargin);
    _fullWorld.setTopMargin(topMargin);
    _fullWorld.setBottomMargin(bottomMargin);
    _fullWorld.setColorScaleWidth(colorScaleWidth);
    _fullWorld.setWorldLimits(-_maxRangeKm, -_maxRangeKm, _maxRangeKm, _maxRangeKm);
    _fullWorld.setXAxisTickLen(axisTickLen);
    _fullWorld.setXNTicksIdeal(nTicksIdeal);
    _fullWorld.setAxisTextMargin(textMargin);

  }
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();

  // Initialize the images used for double-buffering.  For some reason,
  // the window size is incorrect at this point, but that will be corrected
  // by the system with a call to resize().

  _refreshImages();
  
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

      // std::vector< PpiBeam* >::iterator beam;
      // for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
      //   (*beam)->setBeingRendered(ifield, true);
      //   field->addBeam(*beam);
      // }
      
    }
    
  } // ifield
  
  // do the rendering

  _performRendering();

  cerr << "UUUUUUUUUUUUUUUUUU66666666666666" << endl;
  update();
}

/*************************************************************************
 * Draw the dividing lines between plots, title etc.
 */

void PolarWidget::_drawDividers(QPainter &painter)
{

  cerr << "CCCCCCCCCCCCCCCCCCCCCCCCC width, height: " << width() << ", " << height() << endl;

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

  // plot panel lower borders
  
  for (int irow = 1; irow < _nRows; irow++) {
    double yy = _titleMargin + irow * _plotHeight;
    QLineF lowerBoundary(0, yy, _plotsGrossWidth, yy);
    painter.drawLine(lowerBoundary);
    cerr << "***** irow, yy: " << irow << ", " << yy << endl;
  }
  
  // plot panel right borders
  
  for (int icol = 1; icol < _nCols; icol++) {
    double xx = icol * _plotWidth;
    QLineF rightBoundary(xx, _titleMargin, xx, height());
    painter.drawLine(rightBoundary);
    cerr << "***** icol, xx: " << icol << ", " << xx << endl;
  }

  // color scale left boundary
  {
    double xx = _plotsGrossWidth;
    QLineF boundary(xx, _titleMargin, xx, height());
    painter.drawLine(boundary);
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

  cerr << "UUUUUUUUUUUUUUUU7777777777777777" << endl;
  //   update();

}

/*************************************************************************
 * _drawOverlays()
 */

void PolarWidget::_drawOverlays(QPainter &painter)
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

      cerr << "1111111 ringRange: " << ringRange << endl;
      cerr << "1111111 maxRangeKm: " << _maxRangeKm << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << ringRange << ", " << -_maxRangeKm << ", "
           << ringRange << ", " << _maxRangeKm << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << -ringRange << ", " << -_maxRangeKm << ", "
           << -ringRange << ", " << _maxRangeKm << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << -_maxRangeKm << ", " << ringRange << ", "
           << _maxRangeKm << ", " << ringRange << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << -_maxRangeKm << ", " << -ringRange << ", "
           << _maxRangeKm << ", " << -ringRange << endl;

      _zoomWorld.drawLine(painter, ringRange-50, -_maxRangeKm-50, ringRange-50, _maxRangeKm-50);
      _zoomWorld.drawLine(painter, -ringRange-50, -_maxRangeKm-50, -ringRange-50, _maxRangeKm-50);
      _zoomWorld.drawLine(painter, -_maxRangeKm-50, ringRange-50, _maxRangeKm-50, ringRange-50);
      _zoomWorld.drawLine(painter, -_maxRangeKm-50, -ringRange-50, _maxRangeKm-50, -ringRange-50);
      
      maxRingRange = ringRange;
      ringRange += _ringSpacing;
    }
    painter.restore();

    _zoomWorld.specifyXTicks(-maxRingRange, _ringSpacing);
    _zoomWorld.specifyYTicks(-maxRingRange, _ringSpacing);

    _zoomWorld.drawAxisLeft(painter, "km", true, true, true, false);
    _zoomWorld.drawAxisRight(painter, "km", true, true, true, false);
    _zoomWorld.drawAxisTop(painter, "km", true, true, true, false);
    _zoomWorld.drawAxisBottom(painter, "km", true, true, true, false);
    
    _zoomWorld.unspecifyXTicks();
    _zoomWorld.unspecifyYTicks();

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

  const DisplayField &field = _manager.getSelectedField();
  _zoomWorld.drawColorScale(field.getColorMap(), painter,
                            _params.label_font_size);

  if (_archiveMode) {

    // add legends with time, field name and elevation angle

    vector<string> legends;
    char text[1024];

    // time legend

    sprintf(text, "Start time: %s", _plotStartTime.asString(0).c_str());
    legends.push_back(text);
    
    // radar and site name legend

    string radarName(_platform.getInstrumentName());
    if (_params.override_radar_name) {
      radarName = _params.radar_name;
    }
    string siteName(_platform.getInstrumentName());
    if (_params.override_site_name) {
      siteName = _params.site_name;
    }
    string radarSiteLabel = radarName;
    if (siteName.size() > 0 && siteName != radarName) {
      radarSiteLabel += "/";
      radarSiteLabel += siteName;
    }
    legends.push_back(radarSiteLabel);

    // field name legend

    string fieldName = _fieldRenderers[_selectedField]->getField().getLabel();
    sprintf(text, "Field: %s", fieldName.c_str());
    legends.push_back(text);

    // elevation legend

    // sprintf(text, "Elevation(deg): %.2f", _meanElev);
    // legends.push_back(text);

    // nrays legend

    // sprintf(text, "NRays: %g", _nRays);
    // legends.push_back(text);
    
    painter.save();
    painter.setPen(Qt::yellow);
    painter.setBrush(Qt::black);
    painter.setBackgroundMode(Qt::OpaqueMode);

    switch (_params.ppi_main_legend_pos) {
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

/////////////////////////////////////////////////////////////	
// Title
    
void PolarWidget::_drawMainTitle(QPainter &painter) 

{

  painter.save();

  // set the font and color
  
  QFont font = painter.font();
  font.setPointSizeF(_params.main_title_font_size);
  painter.setFont(font);
  painter.setPen(_params.main_title_color);

  string title("CONDOR POLAR PLOTS");

#ifdef NOTNOW
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
#endif

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


////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *PolarWidget::_getClosestRay(double x_km, double y_km)

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

void PolarWidget::ExamineEdit(const RadxRay *closestRay) 

{
  

  // get an version of the ray that we can edit
  // we'll need the az, and sweep number to get a list from
  // the volume

  // NOTE - this will need to be done in  the plot object

  RadxRay *closestRayToEdit = NULL;

#ifdef NOTNOW
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
        LOG(DEBUG) << "Found closest ray: index = " << idx << " pointer = " << closestRayToEdit;
        closestRay->print(cout); 
      }
    }
    r += 1;
    idx += 1;
  }  
  if (!foundIt || closestRayToEdit == NULL)
    throw "couldn't find closest ray";
#endif

  
  //RadxRay *closestRayCopy = new RadxRay(*closestRay);

  // create the view
  SpreadSheetView *sheetView;
  sheetView = new SpreadSheetView(this, closestRayToEdit->getAzimuthDeg());

  // create the model

  // SpreadSheetModel *model = new SpreadSheetModel(closestRayCopy);
  RadxVol *_vol = NULL; // NEED get from plot
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

void PolarWidget::contextMenuEditor()
{
  LOG(DEBUG) << "enter";

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
    ExamineEdit(closestRay);
  }
  LOG(DEBUG) << "exit";
}

/* TODO add to PolarWidget class
   void PolarWidget::errorMessage(string title, string message) {
   QMessageBox::information(this, QString::fromStdString(title), QString::fromStdString(message));
   }
*/

void PolarWidget::ShowContextMenu(const QPoint &pos, RadxVol *vol)
{

  // RadxVol _vol = NULL; // need to sort out with plots
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

#ifdef NOTNOW  

void PolarWidget::contextMenuParameterColors()

{

  LOG(DEBUG) << "enter";

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
u			  annotation_color,
			  _params.background_color);
  FieldColorController *fieldColorController = new FieldColorController(parameterColorView, displayFieldModel);
  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  //  connect(parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
  connect(fieldColorController, SIGNAL(colorMapRedefineSent(string, ColorMap, QColor, QColor, QColor, QColor)),
  	  &_manager, SLOT(colorMapRedefineReceived(string, ColorMap, QColor, QColor, QColor, QColor))); // THIS IS NOT CALLED!!
  //  PolarManager::colorMapRedefineReceived(string, ColorMap)
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
 
  LOG(DEBUG) << "exit ";

}

#endif
  
