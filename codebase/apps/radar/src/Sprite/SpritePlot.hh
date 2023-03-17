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
// SpritePlot.hh
//
// Base class for plots in Sprite
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2023
//
///////////////////////////////////////////////////////////////
#ifndef SpritePlot_HH
#define SpritePlot_HH

#include <string>
#include <vector>

#include <QWidget>
#include <QPainter>

#include "Params.hh"
#include "WorldPlot.hh"

class Beam;
class MomentsFields;

class SpritePlot
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * Constructor.
   */
  
  SpritePlot(QWidget *parent,
             const Params &params,
             int id);
  
  /**
   * @brief Destructor.
   */

  virtual ~SpritePlot();

  /**
   * Clear the plot
   */
  
  virtual void clear() = 0;

  // set the window geometry
  
  virtual void setWindowGeom(int width,
                             int height,
                             int xOffset,
                             int yOffset);
  
  // set the world limits

  virtual void setWorldLimits(double xMinWorld,
                              double yMinWorld,
                              double xMaxWorld,
                              double yMaxWorld);

  // set the zoom limits, using pixel space
  
  virtual void setZoomLimits(int xMin,
                             int yMin,
                             int xMax,
                             int yMax);
  
  virtual void setZoomLimitsX(int xMin,
                              int xMax);
  
  virtual void setZoomLimitsY(int yMin,
                              int yMax);

  // zooming

  virtual void zoom(int x1, int y1, int x2, int y2);
  virtual void unzoom();
  
  // set grid lines on/off

  void setXGridLinesOn(bool val) { _xGridLinesOn = val; }
  void setYGridLinesOn(bool val) { _yGridLinesOn = val; }

  // legends
  
  void setLegendsOn(bool val) { _legendsOn = val; }

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
  
  bool getXGridLinesOn() const { return _xGridLinesOn; }
  bool getYGridLinesOn() const { return _yGridLinesOn; }

  // legends

  bool getLegendsOn() const { return _legendsOn; }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  QWidget *_parent;
  const Params &_params;
  int _id;

  // unzoomed world

  WorldPlot _fullWorld;

  // zoomed world

  bool _isZoomed;
  WorldPlot _zoomWorld;

  // grid lines

  bool _xGridLinesOn;
  bool _yGridLinesOn;

  // legends

  bool _legendsOn;
  
};

#endif
