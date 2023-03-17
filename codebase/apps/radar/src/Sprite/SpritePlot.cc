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
// SpritePlot.cc
//
// Base class for plots in Sprite
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2023
//
///////////////////////////////////////////////////////////////

#include "SpritePlot.hh"
#include "Beam.hh"

using namespace std;

SpritePlot::SpritePlot(QWidget* parent,
                       const Params &params,
                       int id) :
        _parent(parent),
        _params(params),
        _id(id)
        
{
  _isZoomed = false;
  _xGridLinesOn = _params.x_grid_lines_on;
  _yGridLinesOn = _params.y_grid_lines_on;
  _legendsOn = true;
}

/*************************************************************************
 * Destructor
 */

SpritePlot::~SpritePlot()
{

}


/*************************************************************************
 * clear()
 */

void SpritePlot::clear()
{

}

/*************************************************************************
 * set the geometry - unzooms
 */

void SpritePlot::setWindowGeom(int width, int height,
                               int xOffset, int yOffset)
{
  _fullWorld.setWindowGeom(width, height, xOffset, yOffset);
  _zoomWorld = _fullWorld;
}

/*************************************************************************
 * set the world limits - unzooms
 */

void SpritePlot::setWorldLimits(double xMinWorld,
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

void SpritePlot::setZoomLimits(int xMin,
                               int yMin,
                               int xMax,
                               int yMax)
{
  _zoomWorld.setZoomLimits(xMin, yMin, xMax, yMax);
  _isZoomed = true;
}

void SpritePlot::setZoomLimitsX(int xMin,
                                int xMax)
{
  _zoomWorld.setZoomLimitsX(xMin, xMax);
  _isZoomed = true;
}

void SpritePlot::setZoomLimitsY(int yMin,
                                int yMax)
{
  _zoomWorld.setZoomLimitsY(yMin, yMax);
  _isZoomed = true;
}

/*************************************************************************
 * perform zoom
 */

void SpritePlot::zoom(int x1, int y1, int x2, int y2)
{

  _zoomWorld.setZoomLimits(x1, y1, x2, y2);
  _isZoomed = true;

}

/*************************************************************************
 * unzoom the view
 */

void SpritePlot::unzoom()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;

}

