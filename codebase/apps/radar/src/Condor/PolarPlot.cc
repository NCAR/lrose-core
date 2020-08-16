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
                     const RadxPlatform &platform,
                     const vector<DisplayField *> &fields,
                     bool haveFilteredFields) :
        _parent(parent),
        _manager(manager),
        _params(params),
        _id(id),
        _platform(platform),
        _fields(fields),
        _haveFilteredFields(haveFilteredFields),
        _selectedField(0),
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
    fieldRenderer->createImage(100, 100);
    _fieldRenderers.push_back(fieldRenderer);
  }

}

/*************************************************************************
 * Destructor
 */

PolarPlot::~PolarPlot()
{

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

/*************************************************************************
 * plot a beam
 */

void PolarPlot::plotBeam(QPainter &painter,
                         Beam *beam,
                         double selectedRangeKm)
  
{

#ifdef JUNK
  if (beam == NULL) {
    cerr << "WARNING - PolarPlot::plotBeam() - got NULL beam, ignoring"
         << endl;
    return;
  }
  
  if(_params.debug) {
    cerr << "======== Ascope - plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
    cerr << "  Max range: " << beam->getMaxRange() << endl;
  }

  const MomentsFields* fields = beam->getOutFields();
  int nGates = beam->getNGates();
  double startRange = beam->getStartRangeKm();
  double gateSpacing = beam->getGateSpacingKm();

  // first use filled polygons (trapezia)
  
  double xMin = _zoomWorld.getXMinWorld();
  QBrush brush(_params.ascope_fill_color);
  brush.setStyle(Qt::SolidPattern);
  
  for (int ii = 1; ii < nGates; ii++) {
    double rangePrev = startRange + gateSpacing * (ii-1);
    double range = startRange + gateSpacing * (ii);
    double valPrev = getFieldVal(_momentType, fields[ii-1]);
    double val = getFieldVal(_momentType, fields[ii]);
    if (val > -9990 && valPrev > -9990) {
      _zoomWorld.fillTrap(painter, brush,
                          xMin, rangePrev,
                          valPrev, rangePrev,
                          val, range,
                          xMin, range);
    }
  }

  // draw the reflectivity field vs range - as line

  painter.save();
  painter.setPen(_params.ascope_line_color);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nGates; ii++) {
    double range = startRange + gateSpacing * ii;
    double val = getFieldVal(_momentType, fields[ii]);
    if (val > -9990) {
      QPointF pt(val, range);
      pts.push_back(pt);
    }
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();

  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the title

  painter.save();
  painter.setPen(_params.ascope_title_color);
  string title("Ascope:");
  title.append(getName(_momentType));
  _zoomWorld.drawTitleTopCenter(painter, title);
  painter.restore();

#endif

}

/*************************************************************************
 * set the geometry - unzooms
 */

void PolarPlot::setWindowGeom(int width, int height,
                              int xOffset, int yOffset)
{
  _fullWorld.setWindowGeom(width, height, xOffset, yOffset);
  _zoomWorld = _fullWorld;
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
 * set the zoom limits, from pixel space
 */

void PolarPlot::setZoomLimits(int xMin,
                              int yMin,
                              int xMax,
                              int yMax)
{
  _zoomWorld.setZoomLimits(xMin, yMin, xMax, yMax);
  _isZoomed = true;
}

void PolarPlot::setZoomLimitsX(int xMin,
                               int xMax)
{
  _zoomWorld.setZoomLimitsX(xMin, xMax);
  _isZoomed = true;
}

void PolarPlot::setZoomLimitsY(int yMin,
                               int yMax)
{
  _zoomWorld.setZoomLimitsY(yMin, yMax);
  _isZoomed = true;
}

/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * Draw the overlays, axes, legends etc
 */

void PolarPlot::_drawOverlays(QPainter &painter, double selectedRangeKm)
{

  // save painter state
  
  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  painter.setPen(_params.axes_label_color);

  // _zoomWorld.drawAxisBottom(painter, getUnits(_momentType),
  //                           true, true, true, _xGridLinesOn);

  // _zoomWorld.drawAxisLeft(painter, "km", 
  //                         true, true, true, _yGridLinesOn);

  // _zoomWorld.drawYAxisLabelLeft(painter, "Range");

  // selected range line
  
  // painter.setPen(_params.ascope_selected_range_color);
  _zoomWorld.drawLine(painter,
                      _zoomWorld.getXMinWorld(), selectedRangeKm,
                      _zoomWorld.getXMaxWorld(), selectedRangeKm);

  painter.restore();

}

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
    if (ii != _selectedField) {
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
  _refreshImages();
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
 * displayImage()
 */

void PolarPlot::displayImage(const size_t field_num)
{
  // If we weren't rendering the current field, do nothing
  if (field_num != _selectedField) {
    return;
  }
  _parent->update();
}


/*************************************************************************
 * perform the rendering
 */

void PolarPlot::_performRendering()
{

  // cerr << "22222222222222222 _selectedField: " << _selectedField << endl;
  // cerr << "22222222222222222 _fieldRenderers.size(): " << _fieldRenderers.size() << endl;
  
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

  _parent->update();

}

/*************************************************************************
 * _refreshImages()
 * NOTE - belongs in PpiPlot
 */

void PolarPlot::_refreshImages()
{

  cerr << "YYYYYYYYYYYYYYYYYYYYYYYY" << endl;
  
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    
    FieldRenderer *field = _fieldRenderers[ifield];
    
    // If needed, create new image for this field
    
    if (_parent->size() != field->getImage()->size()) {
      field->createImage(_parent->width(), _parent->height());
    }

    // clear image

    field->getImage()->fill(_backgroundBrush.color().rgb());
    
    // set up rendering details

    field->setTransform(_zoomTransform);
    
    // Add pointers to the beams to be rendered
    
    if (ifield == _selectedField || field->isBackgroundRendered()) {

      std::vector< PpiBeam* >::iterator beam;
      // for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
      //   (*beam)->setBeingRendered(ifield, true);
      //   field->addBeam(*beam);
      // }
      
    }
    
  } // ifield
  
  // do the rendering

  _performRendering();

  _parent->update();

}

////////////////////
// set the transform

void PolarPlot::_setTransform(const QTransform &transform)
{
  float worldScale = _zoomWorld.getXMaxWindow() - _zoomWorld.getXMinWindow();
  BoundaryPointEditor::Instance()->setWorldScale(worldScale);
  _fullTransform = transform;
  _zoomTransform = transform;
}
  
