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
/////////////////////////////////////////////////////////////
// PolarPlot.cc
//
// Plotting for power vs range in an ascope
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////

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

#include <Radx/RadxPlatform.hh>
#include <Radx/RadxVol.hh>

#include "PolarPlot.hh"
#include "PolarManager.hh"
#include "Beam.hh"

using namespace std;

const double PolarPlot::SIN_45 = sin(45.0 * DEG_TO_RAD);
const double PolarPlot::SIN_30 = sin(30.0 * DEG_TO_RAD);
const double PolarPlot::COS_30 = cos(30.0 * DEG_TO_RAD);


PolarPlot::PolarPlot(PolarWidget *parent,
                     const PolarManager &manager,
                     const Params &params,
                     int id,
                     Params::plot_type_t plotType,
                     string label,
                     double minAz,
                     double maxAz,
                     double minEl,
                     double maxEl,
                     double maxRangeKm,
                     const RadxPlatform &platform,
                     const vector<DisplayField *> &fields,
                     bool haveFilteredFields) :
        _parent(parent),
        _manager(manager),
        _params(params),
        _id(id),
        _plotType(plotType),
        _label(label),
        _minAz(minAz),
        _maxAz(maxAz),
        _minEl(minEl),
        _maxEl(maxEl),
        _maxRangeKm(maxRangeKm),
        _platform(platform),
        _fields(fields),
        _haveFilteredFields(haveFilteredFields),
        _fieldNum(0),
        _scaledLabel(ScaledLabel::DistanceEng)
        
{

  _isZoomed = false;
  _ringsEnabled = false;
  _gridsEnabled = false;
  _angleLinesEnabled = false;

  // mode

  _archiveMode = _params.begin_in_archive_mode;
  
  // create the field renderers

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    FieldRenderer *fieldRenderer =
      new FieldRenderer(_params, ii, *_fields[ii]);
    fieldRenderer->createImage(200, 200);
    _fieldRenderers.push_back(fieldRenderer);
  }

  activateArchiveRendering();

  // colors
  
  setBackgroundColor(QColor(_params.background_color));
  setGridRingsColor(QColor(_params.grid_and_range_ring_color));

  // image
  
  setImageWidth(200);
  setImageHeight(200);
  setImageOffsetX(0);
  setImageOffsetY(0);

  _image = NULL;
  _createImage(_imageWidth, _imageHeight);

}

/*************************************************************************
 * Destructor
 */

PolarPlot::~PolarPlot()
{

  delete _image;

  // Delete all of the field renderers

  for (size_t i = 0; i < _fieldRenderers.size(); ++i) {
    delete _fieldRenderers[i];
  }
  _fieldRenderers.clear();

}


/*************************************************************************
 * clear()
 */

void PolarPlot::clear()
{

}

/*************************************************************************
 * perform zoom
 */

void PolarPlot::zoom(int x1, int y1, int x2, int y2)
{

  _zoomWorld.setZoomLimits(x1, y1, x2, y2);
  _isZoomed = true;

}

/*************************************************************************
 * unzoom the view
 */

void PolarPlot::unzoom()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;

}

/*************************************************************************
 * set archive mode
 */

void PolarPlot::setArchiveMode(bool archive_mode)
{
  _archiveMode = archive_mode;
}

/////////////////////////////////////
// create image into which we render

void PolarPlot::_createImage(int width, int height)

{
  delete _image;
  _image = new QImage(width, height, QImage::Format_RGB32);
}

/*************************************************************************
 * set the geometry - unzooms
 */

void PolarPlot::setWindowGeom(int width, int height,
                              int offsetX, int offsetY)
{

  setImageWidth(width);
  setImageHeight(height);
  setImageOffsetX(offsetX);
  setImageOffsetY(offsetY);

  _createImage(width, height);

  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    FieldRenderer *field = _fieldRenderers[ifield];
    field->createImage(width, height);
  }

  _fullWorld.setWindowGeom(width, height, 0, 0);
  _zoomWorld = _fullWorld;
  _fullTransform = _fullWorld.getTransform();
  _zoomTransform = _zoomWorld.getTransform();

  refreshFieldImages();

}

/*************************************************************************
 * set the world limits - unzooms
 */

void PolarPlot::setWorldLimits(double xMinWorld,
                               double yMinWorld,
                               double xMaxWorld,
                               double yMaxWorld)
{
  _fullWorld.setWorldLimits(xMinWorld, yMinWorld,
                         xMaxWorld, yMaxWorld);
  _zoomWorld = _fullWorld;
}

/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * Draw the overlays, axes, legends etc
 */

// void PolarPlot::_drawOverlays(QPainter &painter, double selectedRangeKm)
// {

//   // save painter state
  
//   painter.save();
  
//   // store font
  
//   QFont origFont = painter.font();
  
//   painter.setPen(_params.axes_label_color);

//   // _zoomWorld.drawAxisBottom(painter, getUnits(_momentType),
//   //                           true, true, true, _xGridLinesOn);

//   // _zoomWorld.drawAxisLeft(painter, "km", 
//   //                         true, true, true, _yGridLinesOn);

//   // _zoomWorld.drawYAxisLabelLeft(painter, "Range");

//   // selected range line
  
//   // painter.setPen(_params.ascope_selected_range_color);
//   _zoomWorld.drawLine(painter,
//                       _zoomWorld.getXMinWorld(), selectedRangeKm,
//                       _zoomWorld.getXMaxWorld(), selectedRangeKm);

//   painter.restore();

// }

/*************************************************************************
 * setRings()
 */

void PolarPlot::setRings(const bool enabled)
{
  _ringsEnabled = enabled;
  _parent->update();
}


/*************************************************************************
 * setGrids()
 */

void PolarPlot::setGrids(const bool enabled)
{
  _gridsEnabled = enabled;
  _parent->update();
}


/*************************************************************************
 * setAngleLines()
 */

void PolarPlot::setAngleLines(const bool enabled)
{
  _angleLinesEnabled = enabled;
  _parent->update();
}


/*************************************************************************
 * turn on archive-style rendering - all fields
 */

void PolarPlot::activateArchiveRendering()
{
  for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
    _fieldRenderers[ii]->setBackgroundRenderingOn();
  }
}


/*************************************************************************
 * turn on reatlime-style rendering - non-selected fields in background
 */

void PolarPlot::activateRealtimeRendering()
{
  
  for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
    if (ii != _fieldNum) {
      _fieldRenderers[ii]->activateBackgroundRendering();
    }
  }

}

/*************************************************************************
 * backgroundColor()
 */

void PolarPlot::setBackgroundColor(const QColor &color)
{
  _backgroundBrush.setColor(color);
  QPalette new_palette = _parent->palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  _parent->setPalette(new_palette);
}


/*************************************************************************
 * gridRingsColor()
 */

void PolarPlot::setGridRingsColor(const QColor &color)
{
  _gridRingsColor = color;
  _parent->update();
}


/*************************************************************************
 * get image for current field
 */

QImage *PolarPlot::getCurrentImage()
{
  *_image = *_fieldRenderers[_fieldNum]->getImage();
  QPainter painter(_image);
  _drawOverlays(painter);
  return _image;
}


/*************************************************************************
 * perform the rendering
 */

void PolarPlot::_performRendering()
{

  // start the rendering threads
  
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    if (ifield == _fieldNum ||
        _fieldRenderers[ifield]->isBackgroundRendered()) {
      _fieldRenderers[ifield]->signalRunToStart();
    }
  } // ifield

  // wait for rendering threads to complete
  
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    if (ifield == _fieldNum ||
        _fieldRenderers[ifield]->isBackgroundRendered()) {
      _fieldRenderers[ifield]->waitForRunToComplete();
    }
  } // ifield

}

////////////////////
// set the transform

void PolarPlot::setTransform(const QTransform &transform)
{
  float worldScale = _zoomWorld.getXMaxWindow() - _zoomWorld.getXMinWindow();
  BoundaryPointEditor::Instance()->setWorldScale(worldScale);
  _fullTransform = transform;
  _zoomTransform = transform;
}
  
