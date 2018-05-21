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
// BdryPolyline
//
// C++ wrapper for boundary polyline data.
//
// Nancy Rehak, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <euclid/Pjg.hh>
#include <rapformats/BdryPolyline.hh>
#include <rapmath/math_macros.h>
#include <toolsa/str.h>

using namespace std;

// constructor

BdryPolyline::BdryPolyline(const int num_secs_extrap,
			   const string &label) :
  _numSecsExtrap(num_secs_extrap),
  _label(label),
  _numSpare(0)
{
}

// destructor

BdryPolyline::~BdryPolyline()
{
}


/************************************************************************
 * assemble() - Load up the given buffer from the object. Handles byte
 *              swapping.
 *
 * Returns true on success, false on failure.
 */

bool BdryPolyline::assemble(MemBuf &mem_buf) const
{
  BDRY_spdb_polyline_t spdb_polyline;
  memset(&spdb_polyline, 0, sizeof(BDRY_spdb_polyline_t));
    
  spdb_polyline.num_pts    = _points.size();
  spdb_polyline.num_secs   = _numSecsExtrap;
  for (int i=0; i<_numSpare; ++i)
  {
    spdb_polyline.spare[i] = _spare[i];
  }
    
  STRcopy(spdb_polyline.object_label, _label.c_str(), BDRY_LABEL_LEN);
    
  _polylineToBE(spdb_polyline);
    
  mem_buf.add((void *)&spdb_polyline,
	      sizeof(BDRY_spdb_polyline_t) - sizeof(BDRY_spdb_point_t));
    
  /*
   * Add each of the points in the polyline to the buffer.
   */

  vector< BdryPoint >::const_iterator point;
    
  for (point = _points.begin(); point != _points.end(); ++point)
    point->assemble(mem_buf);

  return true;
}



/************************************************************************
 * disassemble() - Disassembles a buffer, sets the object values. Handles
 *                 byte swapping.
 *
 * Returns true on success, false on failure.
 */

bool BdryPolyline::disassemble(char *&buf, int &len)
{
  static const string method_name = "BdryPolyline::disassemble()";
  
  int polyline_size = sizeof(BDRY_spdb_polyline_t) - sizeof(BDRY_spdb_point_t);
  if (polyline_size > len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Incoming buffer too small for product" << endl;
    
    return false;
  }

  BDRY_spdb_polyline_t *spdb_polyline = (BDRY_spdb_polyline_t *)buf;
  
  _polylineFromBE(*spdb_polyline);
  
  _numSecsExtrap = spdb_polyline->num_secs;
  _label = spdb_polyline->object_label;
  for (int i=0; i<BDRY_POLYLINE_SPARE_LEN; ++i)
  {
    _spare[i] = spdb_polyline->spare[i];
    if (_spare[i] != 0)
    {
      _numSpare = i+1;
    }
  }
    
  buf += polyline_size;
  len -= polyline_size;
  
  for (int j = 0; j < spdb_polyline->num_pts; j++)
  {
    BdryPoint point;
    if (!point.disassemble(buf, len))
      return false;
    
    _points.push_back(point);
  } /* endfor - j */
  
  return true;
}


/************************************************************************
 * extrapolate(): Extrapolate the polyline using simple extrapolation of
 *                each point in the boundary using the given speed (in m/s)
 *                and direction.
 *
 * Note: This method updates the polyline in place with the resulting
 *       extrapolation locations.
 */

void BdryPolyline::extrapolate(const int extrap_secs,
			       const double speed, const double direction)
{
  /*
   * Calculate the distance each point will move in the
   * given extrapolation time.  Note that the detection
   * speed is given in m/s and extrapolation time is given
   * in seconds.
   */

  double extrap_km = (speed * extrap_secs) / 1000.0;
    
  /*
   * Now extrapolate each point in the polyline.
   */

  vector< BdryPoint >::iterator point;
    
  for (point = _points.begin(); point != _points.end(); ++point)
  {
    double extrap_lat, extrap_lon;
      
    Pjg::latlonPlusRTheta(point->getLat(), point->getLon(),
			  extrap_km, direction,
			  extrap_lat, extrap_lon);
      
    point->setLocation(extrap_lat, extrap_lon);

  } /* endfor - point */
      
  return;
}


/************************************************************************
 * extrapPointMotion():  Extrapolate the polyline using the motion vectors
 *                       associated with each point on the polyline.
 *
 * Note: This method updates the polyline in place with the resulting
 *       extrapolation locations.
 */

void BdryPolyline::extrapPointMotion(const int extrap_secs)
{
  // Extrapolate each point in the polyline

  vector< BdryPoint >::iterator point;
  
  for (point = _points.begin(); point != _points.end(); ++point)
  { 
    double ucomp = point->getUComp();
    double vcomp = point->getVComp();

    // If the values of u and v are valid, move the point accordingly.
    // If the values are not valid, do not move the point at all, i.e.
    // do nothing.
    
    if (ucomp != BDRY_VALUE_UNKNOWN && vcomp != BDRY_VALUE_UNKNOWN)
    {
      // Find the speed and direction of the boundary at this
      // point.  Note that speed is in m/s and direction is
      // in radar degrees.

      double bdry_pt_speed, bdry_pt_dir;
	
      _uv2SpeedDir(ucomp, vcomp, bdry_pt_speed, bdry_pt_dir);
	
      // Calculate the distance the point will move in the
      // given extrapolation time.  

      double extrap_km = (bdry_pt_speed * extrap_secs) / 1000.0;
      double extrap_lat, extrap_lon;
	
      Pjg::latlonPlusRTheta(point->getLat(), point->getLon(),
			    extrap_km, bdry_pt_dir,
			    extrap_lat, extrap_lon);
	
      point->setLocation(extrap_lat, extrap_lon);
    } /* endif */
	
  } /* endfor - point */
      
  return;
}


/************************************************************************
 * print(): Print the boundary polyline to the given stream.
 */

void BdryPolyline::print(FILE *stream, const bool print_points) const
{
  fprintf(stream, "\nBoundary Polyline:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   num_pts = %d\n", (int) _points.size());
  fprintf(stream, "   num_secs = %d\n", _numSecsExtrap);
  fprintf(stream, "   object_label = <%s>\n", _label.c_str());

  if (print_points)
  {
    vector< BdryPoint >::const_iterator point;
    
    for (point = _points.begin(); point != _points.end(); ++point)
    {
      point->print(stream);
      fprintf(stream, "\n");
    }
  }
}

void BdryPolyline::print(ostream &stream, const bool print_points) const
{
  stream << endl;
  stream << "Boundary Polyline:" << endl;
  stream << endl;
  stream << "   num_pts = " << _points.size() << endl;
  stream << "   num_secs = " << _numSecsExtrap << endl;
  stream << "   object_label = <" << _label << ">" << endl;

  if (print_points)
  {
    vector< BdryPoint >::const_iterator point;
    
    for (point = _points.begin(); point != _points.end(); ++point)
    {
      point->print(stream);
      stream << endl;
    }
  }
}


/************************************************************************
 * STATIC ROUTINES
 ************************************************************************/

/************************************************************************
 * _polylineFromBE(): Convert a boundary polyline structure from big-endian
 *                    format to native format.
 */

void BdryPolyline::_polylineFromBE(BDRY_spdb_polyline_t &polyline)
{
  polyline.num_pts       = BE_to_si32(polyline.num_pts);
  polyline.num_secs      = BE_to_si32(polyline.num_secs);
  BE_to_array_32(polyline.spare, sizeof(polyline.spare));
  /* object_label is okay */

  return;
}


/************************************************************************
 * _polylineToBE(): Convert a boundary polyline structure from native
 *                  format to big-endian format.
 */

void BdryPolyline::_polylineToBE(BDRY_spdb_polyline_t &polyline)
{
  polyline.num_pts       = BE_from_si32(polyline.num_pts);
  polyline.num_secs      = BE_from_si32(polyline.num_secs);
  BE_from_array_32(polyline.spare, sizeof(polyline.spare));
  /* object_label is okay */

  return;
}


/************************************************************************
 * _uv2SpeedDir(): Convert U/V values to speed and direction clockwise
 *                 from true north (i.e. radar coordinates).
 *
 * Note:  u and v can be in any speed units, direction is returned in
 *        degrees and speed is returned in the same units as u and v.
 */

void BdryPolyline::_uv2SpeedDir(const double u, const double v,
				double &speed, double &direction)
{
  double dir_rad;

  // Get the direction of the wind vector.
  // wind dir = inv tan(v/u)  (in polar coordinates)
  // Check special cases first.
  
  if (u == 0.0)
  {
    if (v == 0.0)
      dir_rad = 0.0;
    else if (v < 0)
      dir_rad = PI * -1/2.0;
    else
      dir_rad = PI/2.0;
  }
  else if (v == 0.0)
  {
    if (u < 0)
      dir_rad = PI;
    else
      dir_rad = 0.0;
  }
  else
  {
    dir_rad = atan2(v, u);
  }
  
  // Convert to degrees.
  // 1 radian = (180/pi) degrees

  direction = dir_rad * RAD_TO_DEG;

  // Convert polar to "radar" coordinates.

  direction = 90.0 - direction;

  // Put in range [0.0, 360)

  while (direction < 0.0)
    direction += 360.0;

  // Get the speed of the wind vector.
  // wind speed = sqrt(u^2 + v^2)

  speed = sqrt(u*u + v*v);
}
