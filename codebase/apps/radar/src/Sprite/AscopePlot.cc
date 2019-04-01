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
// AscopePlot.cc
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

#include "AscopePlot.hh"
#include "SpectraMgr.hh"
#include "Beam.hh"

using namespace std;

AscopePlot::AscopePlot(QWidget* parent,
                       const Params &params) :
        _parent(parent),
        _params(params)
        
{
  
}

/*************************************************************************
 * Destructor
 */

AscopePlot::~AscopePlot()
{

}


/*************************************************************************
 * clear()
 */

void AscopePlot::clear()
{

}

/*************************************************************************
 * unzoom the view
 */

void AscopePlot::unzoomView()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _parent->update();

}

////////////////////
// set the transform

void AscopePlot::_setTransform(const QTransform &transform)
{
  
  _fullTransform = transform;
  _zoomTransform = transform;
  
}
  
/*************************************************************************
 * plot a beam
 */

void AscopePlot::plotBeam(QPainter &painter,
                          Beam *beam,
                          bool xGridEnabled,
                          bool yGridEnabled)
  
{

  if (beam == NULL) {
    cerr << "WARNING - AscopePlot::plotBeam() - got NULL beam, ignoring"
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
  
  double xMin = _fullWorld.getXMinWorld();
  QBrush brush(_params.ascope_fill_color);
  brush.setStyle(Qt::SolidPattern);
  
  for (int ii = 1; ii < nGates; ii++) {
    double rangePrev = startRange + gateSpacing * (ii-1);
    double range = startRange + gateSpacing * (ii);
    double valPrev = getFieldVal(_momentType, fields[ii-1]);
    double val = getFieldVal(_momentType, fields[ii]);
    if (val > -9990 && valPrev > -9990) {
      _fullWorld.fillTrap(painter, brush,
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
  _fullWorld.drawLines(painter, pts);
  painter.restore();

  // draw the overlays

  _drawOverlays(painter, xGridEnabled, yGridEnabled);

  // draw the title

  string title("Ascope:");
  title.append(momentTypeStr(_momentType));
  _fullWorld.drawTitleTopCenter(painter, title);

}

//////////////////////////////////
// get a string for the field type

string AscopePlot::momentTypeStr(Params::moment_type_t mtype)
{
  switch (mtype) {
    case Params::DBZ:
      return "DBZ";
    case Params::VEL:
      return "VEL";
    case Params::WIDTH:
      return "WIDTH";
    case Params::NCP:
      return "NCP";
    case Params::SNR:
      return "SNR";
    case Params::DBM:
      return "DBM";
    case Params::ZDR:
      return "ZDR";
    case Params::LDR:
      return "LDR";
    case Params::RHOHV:
      return "RHOHV";
    case Params::PHIDP:
      return "PHIDP";
    case Params::KDP:
      return "KDP";
    default:
      return "UNKNOWN";
  }
}

////////////////////////////////////////////
// get a field value based on the field type

double AscopePlot::getFieldVal(Params::moment_type_t mtype,
                               const MomentsFields &fields)
{
  switch (mtype) {
    case Params::DBZ:
      return fields.dbz;
    case Params::VEL:
      return fields.vel;
    case Params::WIDTH:
      return fields.width;
    case Params::NCP:
      return fields.ncp;
    case Params::SNR:
      return fields.snr;
    case Params::DBM:
      return fields.dbm;
    case Params::ZDR:
      return fields.zdr;
    case Params::LDR:
      return fields.ldr;
    case Params::RHOHV:
      return fields.rhohv;
    case Params::PHIDP:
      return fields.phidp;
    case Params::KDP:
      return fields.kdp;
    default:
      return -9999.0;
  }
}

/*************************************************************************
 * set the geometry
 */

void AscopePlot::setWindowGeom(int width, int height,
                               int xOffset, int yOffset)
{

  _fullWorld.setWindowGeom(width, height, xOffset, yOffset);
  _zoomWorld = _fullWorld;

}

/*************************************************************************
 * set the world limits
 */

void AscopePlot::setWorldLimits(double xMinWorld,
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

void AscopePlot::_drawOverlays(QPainter &painter,
                               bool xGridEnabled,
                               bool yGridEnabled)
{

  // save painter state
  
  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  _zoomWorld.drawAxisBottom(painter, "dBZ", 
                            true, true, true, xGridEnabled);
  _zoomWorld.drawAxisLeft(painter, "km", 
                          true, true, true, yGridEnabled);

  painter.restore();

#ifdef JUNK
  
  // Set the painter to use the right color and font

  painter.setPen(_params.ascope_axes_color);
  
  // axes and labels

  QFont font(origFont);
  font.setPointSizeF(_params.ascope_axis_label_font_size);
  painter.setFont(font);
  // painter.setWindow(0, 0, width(), height());

  // axes

  QColor lineColor(_params.ascope_axes_color);
  QColor gridColor(_params.ascope_grid_color);
  QColor textColor(_params.ascope_labels_color);

  QFont labelFont(origFont);
  labelFont.setPointSizeF(_params.ascope_axis_label_font_size);
  QFont valuesFont(origFont);
  valuesFont.setPointSizeF(_params.ascope_axis_values_font_size);
  
  // y label

  painter.setPen(_params.ascope_labels_color);
  _zoomWorld.drawYAxisLabelLeft(painter, "Amplitude (**)");
  
  // legends
  
  vector<string> legends;
  char text[1024];
  sprintf(text, "Legend1: %g", 1.0);
  legends.push_back(text);
  sprintf(text, "Legend2 lon: %g", 2.0);
  legends.push_back(text);

  if (_params.ascope_plot_legend1) {
    switch (_params.ascope_legend1_pos) {
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
    
  if (_params.ascope_plot_legend2) {
    switch (_params.ascope_legend2_pos) {
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
    
  // title
    
  font.setPointSizeF(_params.ascope_title_font_size);
  painter.setFont(font);

  string radarName(_params.radar_name);
  string title;
  title = (radarName + "   ASCOPE   ");
  _zoomWorld.drawTitleTopCenter(painter, title);

  _zoomWorld.drawAxesBox(painter);

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

  // const DisplayField &field = _manager.getSelectedField();
  // _zoomWorld.drawColorScale(field.getColorMap(), painter,
  //                           _params.ascope_axis_label_font_size);
  
  return;

#endif
  
}

/*************************************************************************
 * call the renderers for each field
 */

void AscopePlot::_performRendering()
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

}


