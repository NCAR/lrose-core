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
#include "PpiPlot.hh"
#include "RhiPlot.hh"

using namespace std;


const double PolarWidget::SIN_45 = sin(45.0 * DEG_TO_RAD);
const double PolarWidget::SIN_30 = sin(30.0 * DEG_TO_RAD);
const double PolarWidget::COS_30 = cos(30.0 * DEG_TO_RAD);

PolarWidget::PolarWidget(QWidget* parent,
                         const PolarManager &manager,
                         const Params &params,
                         const RadxPlatform &platform,
                         const vector<DisplayField *> &fields) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(params),
        _platform(platform),
        _fields(fields),
        _fieldNum(0),
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

  _dividerWidth = _params.main_window_panel_divider_line_width;

  _titleHeight = _params.main_window_title_margin;
  _colorScaleWidth = _params.color_scale_width;
  _plotsTopY = _titleHeight;
  _aspectRatio = _params.polar_plot_aspect_ratio;
  _fullWorld.setColorScaleWidth(_colorScaleWidth);
  _fullWorld.setTopMargin(_titleHeight);
  _fullWorld.setBackgroundColor(_params.background_color);

  _nPlots = _params.polar_plots_n;
  if (_nPlots < _params.polar_plots_n_columns) {
    _nCols = _params.polar_plots_n;
    _nRows = 1;
  } else {
    _nCols = _params.polar_plots_n_columns;
    _nRows = (_nPlots - 1) / _nCols + 1;
  }

  _titleImage = NULL;
  _colorScaleImage = NULL;
  _colorScaleWorld.setWindowGeom(200, 200, 0, 0);
  _colorScaleWorld.setColorScaleWidth(_colorScaleWidth);
  _colorScaleWorld.setLeftMargin(5);
  _colorScaleWorld.setRightMargin(5);
  _colorScaleWorld.setTopMargin(10);
  _colorScaleWorld.setBottomMargin(10);

  _plotsSumHeight = height() - _titleHeight - 1;
  _plotsSumWidth = width() - _colorScaleWidth - 1;
  _plotWidth = _plotsSumWidth / _nCols;
  _plotHeight = _plotsSumHeight / _nRows;

  _ppiPlotsConfigured = false;
  _rhiPlotsConfigured = false;

  _xGridEnabled = false;
  _yGridEnabled = false;

  _currentRay = NULL;

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
  _openingFileInfoLabel->setStyleSheet("QLabel { background-color : darkBlue; "
                                       "color : yellow; "
                                       "qproperty-alignment: AlignCenter; }");
  _openingFileInfoLabel->setVisible(false);

  // create plots

  for (int iplot = 0; iplot < _nPlots; iplot++) {

    const Params::polar_plot_t &plotParams = _params._polar_plots[iplot];

    if (plotParams.plot_type == Params::PPI_PLOT) {
      
      PpiPlot *ppi = new PpiPlot(this, _manager, _params, iplot,
                                 plotParams.plot_type,
                                 plotParams.label,
                                 plotParams.min_az,
                                 plotParams.max_az,
                                 plotParams.min_el,
                                 plotParams.max_el,
                                 plotParams.min_x_km,
                                 plotParams.max_x_km,
                                 plotParams.min_y_km,
                                 plotParams.max_y_km,
                                 _platform,
                                 fields);

      _ppis.push_back(ppi);
      _plots.push_back(ppi);
      
    } else if (plotParams.plot_type == Params::RHI_PLOT) {
      
      RhiPlot *rhi = new RhiPlot(this, _manager, _params, iplot,
                                 plotParams.plot_type,
                                 plotParams.label,
                                 plotParams.min_az,
                                 plotParams.max_az,
                                 plotParams.min_el,
                                 plotParams.max_el,
                                 plotParams.min_x_km,
                                 plotParams.max_x_km,
                                 plotParams.min_y_km,
                                 plotParams.max_y_km,
                                 _platform,
                                 fields);

      _rhis.push_back(rhi);
      _plots.push_back(rhi);
      
    }
    
  } // iplot

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


// set current field

void PolarWidget::setFieldNum(int fieldNum)

{
  _fieldNum = fieldNum;
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->setFieldNum(_fieldNum);
  }
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
  _refreshFieldImages();
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->unzoom();
  }
}


/*************************************************************************
 * clear the data and images of the plots
 */

void PolarWidget::clear()
{
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->clear();
  }
}


/*************************************************************************
 * setRings()
 */

void PolarWidget::setRings(bool enabled)
{
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->setRings(enabled);
  }
  update();
}


/*************************************************************************
 * setGrids()
 */

void PolarWidget::setGrids(const bool enabled)
{
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->setGrids(enabled);
  }
  update();
}


/*************************************************************************
 * setAngleLines()
 */

void PolarWidget::setAngleLines(const bool enabled)
{
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->setAngleLines(enabled);
  }
  update();
}


/*************************************************************************
 * turn on archive-style rendering - all fields
 */

void PolarWidget::activateArchiveRendering()
{
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->activateArchiveRendering();
  }
}


/*************************************************************************
 * turn on reatlime-style rendering - non-selected fields in background
 */

void PolarWidget::activateRealtimeRendering()
{
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->activateRealtimeRendering();
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
  _refreshFieldImages();
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
    _mousePressX = e->position().x();
    _mousePressY = e->position().y();
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);
    emit customContextMenuRequested(clickPos.toPoint());
  } else {
    _rubberBand->setGeometry(QRect(e->pos(), QSize()));
    _rubberBand->show();
    _mousePressX = e->position().x();
    _mousePressY = e->position().y();
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

  int ix = e->position().x();
  int iy = e->position().y();
  int deltaX = ix - _mousePressX;
  int deltaY = iy - _mousePressY;
  
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
    
    _mousePressX = e->position().x();
    _mousePressY = e->position().y();
    
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
      
      _pointClicked = true;

      int plotId = getPlotIdClicked(_mouseReleaseX, _mouseReleaseY);
      if (plotId >= 0) {

        PolarPlot *plot = _plots[plotId];
        int ixx = _mousePressX - plot->getImageOffsetX();
        int iyy = _mousePressY - plot->getImageOffsetY();
        
        // get ray closest to click point
        
        double xKm, yKm;
        const RadxRay *closestRay = plot->getClosestRay(ixx, iyy, xKm, yKm);
        
        // emit signal
        
        emit polarLocationClicked(xKm, yKm, closestRay, plot->getLabel());

      }
      
    } else {
      
      // mouse moved more than 20 pixels, so a zoom occurred
      
      _worldPressX = _zoomWorld.getXWorld(_mousePressX);
      _worldPressY = _zoomWorld.getYWorld(_mousePressY);
      
      _worldReleaseX = _zoomWorld.getXWorld(_zoomCornerX);
      _worldReleaseY = _zoomWorld.getYWorld(_zoomCornerY);

      int plotIdStart = getPlotIdClicked(_mousePressX, _mousePressY);
      int plotIdEnd = getPlotIdClicked(_mouseReleaseX, _mouseReleaseY);

      if (plotIdStart == plotIdEnd && plotIdStart >= 0) {
        
        PolarPlot *plot = _plots[plotIdStart];

        int xx1 = _mousePressX - plot->getImageOffsetX();
        int yy1 = _mousePressY - plot->getImageOffsetY();
        int xx2 = _zoomCornerX - plot->getImageOffsetX();
        int yy2 = _zoomCornerY - plot->getImageOffsetY();

        _plots[plotIdStart]->zoom(xx1, yy1, xx2, yy2);

      }
      
      _manager.enableZoomButton();
      
    }
    
    // hide the rubber band
    
    _rubberBand->hide();

    // paint
    
    update();

  }
}

/*************************************************************************
 * get plot id for a given click location
 *
 * returns plot id on success, -1 on error
 */

int PolarWidget::getPlotIdClicked(int ix, int iy) const
{
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    const PolarPlot *plot = _plots[ii];
    if ((ix >= plot->getImageOffsetX()) &&
        (ix <= plot->getImageOffsetX() + plot->getImageWidth() - 1)  &&
        (iy >= plot->getImageOffsetY()) &&
        (iy <= plot->getImageOffsetY() + plot->getImageHeight() - 1)) {
      return plot->getId();
    }
  }
  return -1;
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
    update();
  }

}

/*************************************************************************
 * handleRay()
 * Handle incomgin ray
 */

void PolarWidget::handleRay(const RadxRay *ray,
                            const std::vector<std::vector<double> > &beam_data,
                            const std::vector<DisplayField*> &fields)
  
{

  _currentRay = ray;

  // add beam to each plot
  
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->addRay(ray, beam_data, fields);
  }
  update();

}

/*************************************************************************
 * paintEvent()
 */

void PolarWidget::paintEvent(QPaintEvent *event)
{

  QPainter painter(this);
  
  // copy in plot images
  
  for (size_t ii = 0; ii < _plots.size(); ii++) {
    QImage *image = _plots[ii]->getCurrentImage();
    painter.drawImage(_plots[ii]->getImageOffsetX(),
                      _plots[ii]->getImageOffsetY(),
                      *image);
  } // ii
  
  // draw color scale and copy in image

  _drawColorScale();
  painter.drawImage(_colorScaleOffsetX, _colorScaleOffsetY,
                    *_colorScaleImage);
  
  // draw title and copy in title image
  
  _drawMainTitle();
  painter.drawImage(_titleOffsetX, _titleOffsetY, *_titleImage);

  // draw the dividers
  
  _drawDividers(painter);
  
}


/*************************************************************************
 * resizeEvent()
 */

void PolarWidget::resizeEvent(QResizeEvent * e)
{
  _resetWorld(width(), height());
  _refreshFieldImages();
}


/*************************************************************************
 * overload resize()
 */

void PolarWidget::resize(int ww, int hh)
{

  // compute the plot geometry, size etc
  
  double plotsSumWidth = ww - _colorScaleWidth - (2 * _nCols) * _dividerWidth;
  double plotsSumHeight = hh - _titleHeight - (2 + _nRows) * _dividerWidth;
  double plotsSumAspect = plotsSumWidth / plotsSumHeight;
  double plotWidth = plotsSumWidth / _nCols;
  double plotHeight = plotsSumHeight / _nRows;

  if (_params.polar_plot_aspect_ratio < 0) {
    // use aspect ratio from window
    _aspectRatio = plotsSumAspect;
  } else {
    // use specified aspect ratio
    _aspectRatio = _params.polar_plot_aspect_ratio;
    if (_aspectRatio > plotsSumAspect) {
      // limit height
      plotHeight = plotWidth / _aspectRatio;
    } else {
      // limit width
      plotWidth = plotHeight * _aspectRatio;
    }
  }
  
  _plotsTopY = _titleHeight;
  _plotWidth = (int) (plotWidth + 0.5);
  _plotHeight = (int) (plotHeight + 0.5);
  _plotsSumWidth = _nCols * _plotWidth;
  _plotsSumHeight = _nRows * _plotHeight;
  int totalWidth = _plotsSumWidth + _colorScaleWidth + (2 + _nCols) * _dividerWidth;
  int totalHeight = _plotsSumHeight + _titleHeight + (2 + _nRows) * _dividerWidth;

  // compute image sizes and locations

  _titleOffsetX = _dividerWidth;
  _titleOffsetY = _dividerWidth;
  _titleHeight = _params.main_window_title_margin;
  _titleWidth = _plotsSumWidth + (_nCols - 1) * _dividerWidth;
  delete(_titleImage);
  _titleImage = new QImage(_titleWidth, _titleHeight, QImage::Format_RGB32);
  
  _colorScaleOffsetX = _titleWidth + 2 * _dividerWidth;
  _colorScaleOffsetY = _dividerWidth;
  _colorScaleWidth = _params.color_scale_width;
  _colorScaleHeight = _titleHeight + _plotsSumHeight + _nRows * _dividerWidth;
  delete(_colorScaleImage);
  _colorScaleImage = new QImage(_colorScaleWidth, _colorScaleHeight, QImage::Format_RGB32);
  _colorScaleWorld.setWindowGeom(_colorScaleWidth, _colorScaleHeight, 0, 0);

  for (size_t iplot = 0; iplot < _plots.size(); iplot++) {
    PolarPlot *plot = _plots[iplot];
    int colNum = iplot % _nCols;
    int rowNum = iplot / _nCols;
    int offsetX = _dividerWidth + colNum * (_plotWidth + _dividerWidth);
    int offsetY = 2 * _dividerWidth + _titleHeight + rowNum * (_plotHeight + _dividerWidth);
    plot->setWindowGeom(_plotWidth, _plotHeight, offsetX, offsetY);
  } // iplot
    
  // resize
  
  setGeometry(0, 0,  totalWidth, totalHeight);

  // repaint
  
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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _fullWorld.print(cerr);
  }

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
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();

  // Initialize the images used for double-buffering.  For some reason,
  // the window size is incorrect at this point, but that will be corrected
  // by the system with a call to resize().

  _refreshFieldImages();
  
}

/*************************************************************************
 * _refreshImages()
 */

void PolarWidget::_refreshFieldImages()
{

  for (size_t ii = 0; ii < _plots.size(); ii++) {
    _plots[ii]->refreshFieldImages();
  }

}

/*************************************************************************
 * Draw the dividing lines between plots, title etc.
 */

void PolarWidget::_drawDividers(QPainter &painter)
{

  // draw panel dividing lines

  painter.save();
  QPen dividerPen(_params.main_window_panel_divider_color);
  dividerPen.setWidth(1);
  // dividerPen.setWidth(_params.main_window_panel_divider_line_width);
  painter.setPen(dividerPen);

  for (int jj = 0; jj < _dividerWidth; jj++) {
    
    // outside borders
    
    QLineF upperBorder(0, jj, width()-1, jj);
    painter.drawLine(upperBorder);
    QLineF lowerBorder(0, height()-1-jj, width()-1, height()-1-jj);
    painter.drawLine(lowerBorder);
    QLineF leftBorder(jj, 0, jj, height()-1);
    painter.drawLine(leftBorder);
    QLineF rightBorder(width()-1-jj, 0, width()-1-jj, height()-1);
    painter.drawLine(rightBorder);
    
    // color scale left boundary

    {
      double xx = _dividerWidth + _titleWidth + jj;
      QLineF leftBoundary(xx, 0, xx, height());
      painter.drawLine(leftBoundary);
    }
    
    // plot panel top borders
    
    for (int irow = 0; irow < _nRows; irow++) {
      int plotNum0 = irow * _nCols;
      int plotImageOffsetY = _plots[plotNum0]->getImageOffsetY();
      int len = _nCols * (_dividerWidth + _plotWidth);
      double yy = plotImageOffsetY-1-jj;
      QLineF topBoundary(0, yy, len + _dividerWidth - 1, yy);
      painter.drawLine(topBoundary);
    }
    
    // plot panel left borders
    
    for (int icol = 0; icol < _nCols; icol++) {
      int plotImageOffsetX = _plots[icol]->getImageOffsetX();
      int plotImageOffsetY = _plots[icol]->getImageOffsetY();
      int len = _nRows * (_dividerWidth + _plotHeight);
      double xx = plotImageOffsetX-1-jj;
      double yy = plotImageOffsetY;
      QLineF leftBoundary(xx, yy, xx, yy + len);
      painter.drawLine(leftBoundary);
    }
    
    // // plot panel right borders
    
    // for (int icol = 1; icol < _nCols; icol++) {
    //   double xx = icol * _plotWidth;
    //   QLineF rightBoundary(xx, _titleHeight, xx, height());
    //   painter.drawLine(rightBoundary);
    // }
    
  } // jj

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

}

/////////////////////////////////////////////////////////////	
// Title
    
void PolarWidget::_drawMainTitle() 

{

  // clear title image

  _titleImage->fill(_backgroundBrush.color().rgb());

  // get painter
  
  QPainter painter(_titleImage);
  painter.save();

  // set the font and color
  
  QFont font = painter.font();
  font.setPointSizeF(_params.main_title_font_size);
  painter.setFont(font);
  painter.setPen(_params.main_title_color);
  
  string title("CONDOR POLAR PLOTS");

  if (_currentRay != NULL) {
    title.append(" - ");
    RadxTime rayTime = _currentRay->getRadxTime();
    title.append(rayTime.asString(3));
  }

  // get bounding rectangle
  
  QRect tRect(painter.fontMetrics().tightBoundingRect(title.c_str()));

  int boxWidth = tRect.width() + 10;
  int boxHeight = tRect.height() + 6;
  qreal xx = (qreal) ((_titleWidth - boxWidth) / 2.0);
  qreal yy = (qreal) ((_titleHeight - boxHeight) / 2.0);
  QRectF bRect(xx, yy, boxWidth, boxHeight);

  // draw the text
  
  painter.drawText(bRect, Qt::AlignTop, title.c_str());

  painter.restore();

}

/////////////////////////////////////////////////////////////	
// color scale
    
void PolarWidget::_drawColorScale()

{

  // clear color scale image
  
  _colorScaleImage->fill(_backgroundBrush.color().rgb());

  // get painter

  QPainter painter(_colorScaleImage);

  // get current field
  
  const DisplayField &field = _manager.getSelectedField();

  // draw color scale

  _colorScaleWorld.drawColorScale(field.getColorMap(),
                                  painter,
                                  _params.label_font_size);

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
  
