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
// AscopePlot.hh
//
// Plotting for power vs range in an ascope
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
#ifndef AscopePlot_HH
#define AscopePlot_HH

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
#include "WorldPlot.hh"

class Beam;
class MomentsFields;

/// AScope plotting

class AscopePlot
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * Constructor.
   */
  
  AscopePlot(QWidget *parent,
             const Params &params);
  
  /**
   * @brief Destructor.
   */

  virtual ~AscopePlot();

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

  // set the moment type

  void setMomentType(Params::moment_type_t val) { _momentType = val; }
  
  // unzoom the view

  void unzoomView();

  // plot a beam
  
  void plotBeam(QPainter &painter,
                Beam *beam,
                bool xGridEnabled,
                bool yGridEnabled);
  
  // get the world plot objects
  
  WorldPlot &getFullWorld() { return _fullWorld; }
  WorldPlot &getZoomWorld() { return _zoomWorld; }
  bool getIsZoomed() const { return _isZoomed; }

  // get the window geom

  int getWidth() const { return _fullWorld.getWidthPixels(); }
  int getHeight() const { return _fullWorld.getHeightPixels(); }
  int getXOffset() const { return _fullWorld.getXPixOffset(); }
  int getYOffset() const { return _fullWorld.getYPixOffset(); }
  
  // get the moment type

  const Params::moment_type_t getMomentType() const { return _momentType; }
  static string getName(Params::moment_type_t mtype);
  static string getUnits(Params::moment_type_t mtype);
  static double getFieldVal(Params::moment_type_t mtype,
                            const MomentsFields &fields);
  static double getMinVal(Params::moment_type_t mtype);
  static double getMaxVal(Params::moment_type_t mtype);
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * parent widget - canvas of which this is part
   */
  
  QWidget *_parent;

  /**
   * @brief TDRP params.
   */

  const Params &_params;

  // range of plot

  double _minRangeKm;
  double _maxRangeKm;

  // moment type

  Params::moment_type_t _momentType;
  
  /**
   * @brief Transform for unzoomed state
   */

  QTransform _fullTransform;
  WorldPlot _fullWorld;
  
  /**
   * @brief Transformed for zoomed state
   */

  bool _isZoomed;
  QTransform _zoomTransform;
  WorldPlot _zoomWorld;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Render the axes, grids, labels and other overlays
   *
   * @param[in] painter    Painter to use for rendering.
   */
  
  void _drawOverlays(QPainter &painter,
                     bool xGridEnabled,
                     bool yGridEnabled);
  
  /**
   * @brief Initialize the full window transform to use for the plot.
   *
   * @param[in] window    The full window to use for the plot.
   */

  void _setTransform(const QTransform &transform);
  
  // reset the pixel size of the world view

  void _resetWorld(int width, int height);
  
  // call the renderers for each field

  void _performRendering();

};

#endif
