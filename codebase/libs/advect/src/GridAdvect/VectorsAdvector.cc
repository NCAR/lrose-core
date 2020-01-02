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
///////////////////////////////////////////////////////////////
// VectorsAdvector.cc
//
// VectorsAdvector class
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#include <advect/VectorsAdvector.hh>
#include <euclid/Pjg.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <math.h>


// Globals

const fl32 VectorsAdvector::MISSING_MOTION_VALUE = -999.0;

//////////////
// Constructor

VectorsAdvector::VectorsAdvector(const double vector_spacing,
				 const double smoothing_radius,
                                 const bool avoid_ghosting,
				 const bool debug_flag) :
  _debugFlag(debug_flag),
  _vectorSpacing(vector_spacing),
  _smoothingRadius(smoothing_radius),
  _gridTemplate(0),
  _avoidGhosting(avoid_ghosting),
  _motionWtData(0),
  _motionUData(0),
  _motionVData(0),
  _initMotionUData(0),
  _initMotionVData(0),
  _initMotionSize(0),
  _soundingUComp(0),
  _soundingVComp(0)
{
  if (_debugFlag) {
    cerr << "In debug mode" << endl;
  }
  _mbuf = MEMbufCreate();
  _nVectors = 0;
  _vectors = (vectors_t *) MEMbufPtr(_mbuf);

  if (_avoidGhosting)
  {
    _soundingUComp = MISSING_MOTION_VALUE;
    _soundingVComp = MISSING_MOTION_VALUE;
  }
  
}

/////////////
// destructor

VectorsAdvector::~VectorsAdvector()

{
  MEMbufDelete(_mbuf);

  delete _gridTemplate;
  
  delete [] _motionWtData;
  delete [] _motionUData;
  delete [] _motionVData;
  delete [] _initMotionUData;
  delete [] _initMotionVData;
}

////////////////////////////
// loadVectors()
//
// Load up the list of vectors to be used
// for the forecast.
//
// Returns true on success, false on failure

bool VectorsAdvector::loadVectors(const Pjg &projection,
				  const fl32 *u_data, const fl32 u_missing,
				  const fl32 *v_data, const fl32 v_missing)
{
  const string method_name = "VectorsAdvector::loadVectors()";
  
  // Clear out the old vectors

  MEMbufReset(_mbuf);
  _nVectors = 0;
  _vectorsPrecomputed = false;
  
  // Load the vectors

  int xSpacing, ySpacing;

  xSpacing = (int)MAX(projection.km2xGrid(_vectorSpacing), 1.0);
  ySpacing = (int)MAX(projection.km2yGrid(_vectorSpacing), 1.0);

  // loop through y and x loading up the vectors

  int nx = projection.getNx();
  int ny = projection.getNy();
  double dx = projection.getDx();
  double dy = projection.getDy();
  double minx = projection.getMinx();
  double miny = projection.getMiny();
  
  for (int iy = ySpacing / 2; iy < ny; iy += ySpacing)
  {
    int i = iy * nx + xSpacing / 2;

    PMU_auto_register("VectorsAdvector::loadVectors ... loading ...");
    
    for (int ix = xSpacing / 2; ix < nx; ix += xSpacing, i += xSpacing)
    {
      fl32 u_value = u_data[i];
      fl32 v_value = v_data[i];

      // Skip missing motion vectors
      
      if (u_value == u_missing || v_value == v_missing)
        continue;

      // compute vector

      vectors_t vec;
      
      vec.u = u_value;
      vec.v = v_value;

      double xx = minx + dx * ix;
      double yy = miny + dy * iy;

      projection.xy2latlon(xx, yy, vec.lat, vec.lon);
	
      // add to membuf
	
      MEMbufAdd(_mbuf, &vec, sizeof(vectors_t));
      _nVectors++;

    } // ix

  } // iy

  // set vector array pointer

  _vectors = (vectors_t *) MEMbufPtr(_mbuf);

  // Initialize the initial motion grids.
  // To avoid ghosting, we initialize the motion grid to missing anywhere that we have a motion vector and to 0
  // anywhere that we don't. When calculating the actual motions, this will keep us from putting values in places
  // where we extrapolated something, but will leave data values in places where we didn't have any vectors. The
  // ghosting happens when we only have vectors in some places, but have missing vectors everywhere else, like when
  // we are just using TITAN vectors to extrapolate a grid.

  size_t grid_size = nx * ny;

  if (_initMotionSize != grid_size)
  {
    cerr << "*** Allocating space for intial motion grids" << endl;
    
    delete [] _initMotionUData;
    delete [] _initMotionVData;

    _initMotionUData = new fl32[grid_size];
    _initMotionVData = new fl32[grid_size];

    _initMotionSize = grid_size;
  }
  
  memset(_initMotionUData, 0, grid_size * sizeof(fl32));
  memset(_initMotionVData, 0, grid_size * sizeof(fl32));
    
  if (_avoidGhosting)
  {
    for (size_t index = 0; index < grid_size; ++index)
    {
      if (u_data[index] != u_missing && v_data[index] != v_missing)
      {
        _initMotionUData[index] = MISSING_MOTION_VALUE;
        _initMotionVData[index] = MISSING_MOTION_VALUE;
      }
    }
    
  }
      
  return true;

}


////////////////////////////
// loadSounding()
//
// Load the sounding to be used for the forecast.  Sounding components
// are used to fill in the missing data spaces in the motion grid when
// the forecast vectors are recomputed in precompute().  The sounding
// components default to 0.0 when this object is created.
//
// Returns true on success, false on failure

bool VectorsAdvector::loadSounding(const double u_comp, const double v_comp)
{
  _soundingUComp = u_comp;
  _soundingVComp = v_comp;
  
  return true;
}

////////////////////////////
// precompute()
//
// Precompute the forecast vectors.
//
// Returns true on success, false on failure

bool VectorsAdvector::precompute(const Pjg &projection,
				 const int lead_time_secs)
{
  int nx = projection.getNx();
  int ny = projection.getNy();
  int old_nx = _motionProjection.getNx();
  int old_ny = _motionProjection.getNy();
  
  // If none of the important information has changed, we are
  // already precomputed.

  if (lead_time_secs == _leadTimeSecs &&
      projection == _motionProjection &&
      _vectorsPrecomputed)
  {
    // Save the new projection in case some of the other information
    // is different.

    _motionProjection = projection;
    
    // Return because we don't need to do any of the calculations

    return true;
  }
  
  // allocate motion grids

  if (nx != old_nx ||
      ny != old_ny)
  {
    delete [] _motionWtData;
    delete [] _motionUData;
    delete [] _motionVData;
    
    _motionWtData = new fl32[nx * ny];
    _motionUData  = new fl32[nx * ny];
    _motionVData  = new fl32[nx * ny];
  }

  size_t motion_grid_size = nx * ny;

  memcpy(_motionUData, _initMotionUData, motion_grid_size * sizeof(fl32));
  memcpy(_motionVData, _initMotionVData, motion_grid_size * sizeof(fl32));
  memset(_motionWtData, 0, motion_grid_size * sizeof(fl32));
  
  // Create the grid template and precompute the distance weights

  if (_gridTemplate == 0 ||
      projection != _motionProjection)
  {
    double major_axis = MAX(2.0 * projection.km2xGrid(_smoothingRadius), 1.0);
    double minor_axis = MAX(2.0 * projection.km2yGrid(_smoothingRadius), 1.0);

    delete _gridTemplate;
    _gridTemplate = new EllipticalTemplate(0.0, major_axis, minor_axis);
    
    vector< GridOffset* > template_points = _gridTemplate->getPointList();
    for (vector< GridOffset* >::iterator pt_iter = template_points.begin();
         pt_iter != template_points.end(); ++ pt_iter)
    {
      GridOffset *offset = *pt_iter;
    
      // Calculate the distance from the template grid point to the
      // center grid point

      double x_dist = projection.xGrid2km((const double)offset->x_offset);
      double y_dist = projection.yGrid2km((const double)offset->y_offset);
    
      double distance = sqrt((x_dist * x_dist) + (y_dist * y_dist));
    
      // velocity components are summmed, weighted inversely with
      // distance from the point, dropping to 0 at smoothing radius
      
      double wt = 1.0 - (distance / _smoothingRadius);

      // Set the weight for this offset object

      offset->value = wt;
    }
  }
  
  // Save the new motion projection and lead time

  _motionProjection = projection;
  _leadTimeSecs = lead_time_secs;

  // load up the grid with forecast motion vectors

  _loadMotionGrid(*_gridTemplate, lead_time_secs);

  _vectorsPrecomputed = true;
  
  return true;
}


////////////////////////////
// calcFcstIndex()
//
// Calculate the grid index of the original grid location from this
// forcast grid location.
//
// Returns the calculated grid index if successful, returns -1 if
// the original location is outside of the grid or if there is no
// motion in that location.

int VectorsAdvector::calcFcstIndex(const int x_index,
				   const int y_index)
{
  int nx = _motionProjection.getNx();
  int ny = _motionProjection.getNy();
  
  int index = x_index + (y_index * nx);
  
  // compute the number of grid points moved

  if (_motionUData[index] == MISSING_MOTION_VALUE ||
      _motionVData[index] == MISSING_MOTION_VALUE)
    return -1;
  
  double x_km = _motionUData[index] * (double)_leadTimeSecs / 1000.0;
  double y_km = _motionVData[index] * (double)_leadTimeSecs / 1000.0;
	
  int x_grid_offset = (int)(_motionProjection.km2xGrid(x_km) + 0.5);
  int y_grid_offset = (int)(_motionProjection.km2yGrid(y_km) + 0.5);
	
  int fcst_x = x_index - x_grid_offset;
  int fcst_y = y_index - y_grid_offset;
	
  if (fcst_x < 0 || fcst_x >= nx ||
      fcst_y < 0 || fcst_y >= ny)
    return -1;
	
  return fcst_x + (fcst_y * nx);
}


/////////////////////
// _loadMotionGrid()
//
// Load up grid with forecast motion vector values.
// The values are placed in the grid at the forecast location rather
// than the starting location, so that we can look BACK to
// find the pixels from which the forecast is constructed.
//
// Returns true on success, false on failure.
//

bool VectorsAdvector::_loadMotionGrid(GridTemplate &grid_template,
				      const int lead_time_secs)
{
  // loop through the vectors

  vectors_t *vecs = _vectors;
  
  for (int i = 0; i < _nVectors; i++, vecs++)
  {
    PMU_auto_register("VectorsAdvector::_loadMotionGrid ... loading ...");
    
    // Compute the ending lat/lon position of each vector

    double speed, direction;
    
    _calcSpeedDirection(vecs->u, vecs->v, speed, direction);
    
    double r = speed * (double)lead_time_secs / 1000.0;
    
    double end_lat, end_lon;
    
    PJGLatLonPlusRTheta(vecs->lat, vecs->lon, r, direction,
			&end_lat, &end_lon);
    
    // load grid for this vector with weighted u,v values

    _loadGridForVector(end_lat, end_lon,
		       vecs->u, vecs->v,
		       grid_template);
    
  } // i

  // Compute the means of the velocity vectors for each grid point

  int nx = _motionProjection.getNx();
  int ny = _motionProjection.getNy();
  
  for (int i = 0; i < nx * ny; i++)
  {
    if (_motionWtData[i] > 0)
    {
      _motionUData[i] /= _motionWtData[i];
      _motionVData[i] /= _motionWtData[i];
    }
    else
    {
      if (_soundingUComp != MISSING_MOTION_VALUE &&
          _soundingVComp != MISSING_MOTION_VALUE)
      {
        _motionUData[i] = _soundingUComp;
        _motionVData[i] = _soundingVComp;
      }
    }
  } // i

  return true;
  
}


///////////////////////////
// _loadGridForVector()
//
// Load up u and v components for grid squares affected by a
// given vector.
//

void VectorsAdvector::_loadGridForVector(const double lat, const double lon,
					 const double u, const double v,
					 GridTemplate &grid_template)
{
  int x_index, y_index;
    
  _motionProjection.latlon2xyIndex(lat, lon, x_index, y_index);
    
  // Loop through the template to calculate the weighted vector

  int nx = _motionProjection.getNx();
  int ny = _motionProjection.getNy();
  
  GridPoint *point;
  
  for (point = grid_template.getFirstInGrid(x_index, y_index,
					    nx, ny);
       point != 0;
       point = grid_template.getNextInGrid())
  {
    int motion_index = point->getIndex(nx, ny);
    
    double wt = point->value;

    if (_motionUData[motion_index] == MISSING_MOTION_VALUE ||
        _motionVData[motion_index] == MISSING_MOTION_VALUE)
    {
      _motionUData[motion_index] = u * wt;
      _motionVData[motion_index] = v * wt;
      _motionWtData[motion_index] = wt;
    }
    else
    {
      _motionUData[motion_index] += u * wt;
      _motionVData[motion_index] += v * wt;
      _motionWtData[motion_index] += wt;
    }

  } /* endfor - point */
  
}


///////////////////////
// _calcSpeedDirection()
//
// Calculate the speed and direction based on the U and V vectors
//

void VectorsAdvector::_calcSpeedDirection(const double u, const double v,
					  double &speed, double &direction)
{
  speed = sqrt((u * u) + (v * v));
  
  if (u == 0.0 || v == 0.0)
    direction = 0.0;
  else
    direction = atan2(u, v) * RAD_TO_DEG;
}
