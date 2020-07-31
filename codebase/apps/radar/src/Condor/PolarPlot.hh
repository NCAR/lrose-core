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
// PolarPlot.hh
//
// Plotting for power vs range in an ascope
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
#ifndef PolarPlot_HH
#define PolarPlot_HH

#include <string>
#include <vector>

#include <QDialog>
#include <QWidget>
#include <QResizeEvent>
#include <QImage>
#include <QTimer>
#include <QRubberBand>
#include <QPoint>
#include <QTransform>

#include "Params.hh"
#include "ScaledLabel.hh"
#include "FieldRenderer.hh"
#include "WorldPlot.hh"
#include "DisplayField.hh"

class Beam;
class MomentsFields;

/// AScope plotting

class PolarPlot
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * Constructor.
   */
  
  PolarPlot(QWidget *parent,
            const Params &params,
            int id,
            const RadxPlatform &platform,
            const vector<DisplayField *> &fields,
            bool haveFilteredFields);
  
  /**
   * @brief Destructor.
   */

  virtual ~PolarPlot();

  /**
   * Clear the plot
   */
  
  void clear();

  // set the window geometry
  
  void setWindowGeom(int width,
                     int height,
                     int xOffset,
                     int yOffset);

  // set the world limits
  
  void setWorldLimits(double xMinWorld,
                      double yMinWorld,
                      double xMaxWorld,
                      double yMaxWorld);

  // set the zoom limits, using pixel space
  
  void setZoomLimits(int xMin,
                     int yMin,
                     int xMax,
                     int yMax);
  
  void setZoomLimitsX(int xMin,
                      int xMax);

  void setZoomLimitsY(int yMin,
                      int yMax);

  // zooming

  void zoom(int x1, int y1, int x2, int y2);
  void unzoom();

  // set archive mode

  void setArchiveMode(bool archive_mode);

  // plot a beam
  
  void plotBeam(QPainter &painter,
                Beam *beam,
                double selectedRangeKm);

  // set overlays

  void setRings(const bool enabled);
  void setGrids(const bool enabled);
  void setAngleLines(const bool enabled);

  // set max range

  virtual void setMaxRange(double max_range) = 0;

  // turn on archive-style rendering - all fields

  void activateArchiveRendering();

  // turn on reatlime-style rendering - non-selected fields in background

  void activateRealtimeRendering();

  // set the background color

  void setBackgroundColor(const QColor &color);

  // set the color for the grid / rings

  void setGridRingsColor(const QColor &color);
  
  // displayImage for given field

  void displayImage(const size_t field_num);

  // get the world plot objects
  
  WorldPlot &getFullWorld() { return _fullWorld; }
  WorldPlot &getZoomWorld() { return _zoomWorld; }
  bool getIsZoomed() const { return _isZoomed; }

  // get the window geom

  int getWidth() const { return _fullWorld.getWidthPixels(); }
  int getHeight() const { return _fullWorld.getHeightPixels(); }
  int getXOffset() const { return _fullWorld.getXPixOffset(); }
  int getYOffset() const { return _fullWorld.getYPixOffset(); }
  
  // get grid lines state

  bool getRingsEnabled() const { return _ringsEnabled; }
  bool getGridsEnabled() const { return _gridsEnabled; }
  bool getAngleLinesEnabled() const { return _angleLinesEnabled; }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  static const double SIN_45;
  static const double SIN_30;
  static const double COS_30;

  QWidget *_parent;
  const Params &_params;
  int _id;

  // unzoomed world

  QTransform _fullTransform;
  WorldPlot _fullWorld;

  // zoomed world

  bool _isZoomed;
  QTransform _zoomTransform;
  WorldPlot _zoomWorld;

  // instrument platform details 

  const RadxPlatform &_platform;
  
  // data fields
  
  const vector<DisplayField *> &_fields;
  bool _haveFilteredFields;
  size_t _selectedField;

  // The renderer for each field.

  vector<FieldRenderer*> _fieldRenderers;
  
  // overlays

  bool _ringsEnabled;
  bool _gridsEnabled;
  bool _angleLinesEnabled;
  double _ringSpacing;

  // geometry

  double _aspectRatio;
  double _maxRangeKm;

  // painting

  QBrush _backgroundBrush;
  QColor _gridRingsColor;
  ScaledLabel _scaledLabel;

  // archive mode

  bool _archiveMode;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Determine a ring spacing which will give even distances, and
  // fit a reasonable number of rings in the display.
  // return Returns the ring spacing in kilometers.

  virtual void _setGridSpacing() = 0;

  // draw the overlays
  
  void _drawOverlays(QPainter &painter, double selectedRangeKm);
  
  // reset the world coords
  
  void _resetWorld(int width, int height);
  
  // rendering
  
  void _performRendering();
  
  // overide refresh images

  virtual void _refreshImages() = 0;

  // get ray closest to click point
  
  virtual const RadxRay *_getClosestRay(double x_km, double y_km) = 0;

};

#endif
