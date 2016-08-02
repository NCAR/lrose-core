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
// PO Box 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#ifndef _BdryPolyline_hh
#define _BdryPolyline_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <dataport/port_types.h>
#include <rapformats/bdry_typedefs.h>
#include <rapformats/BdryPoint.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

class BdryPolyline
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /************************************************************************
   * Constructors
   */

  BdryPolyline(const int num_secs_extrap = 0,
	       const string &label = "");

  /************************************************************************
   * Destructor
   */

  virtual ~BdryPolyline();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /************************************************************************
   * getLabel(): Get the polyline label string.
   */

  inline string getLabel() const
  {
    return _label;
  }
  

  /************************************************************************
   * getNumSecsExtrap(): Get the number of seconds of extrapolation for the
   *                     polyline.
   */

  inline int getNumSecsExtrap() const
  {
    return _numSecsExtrap;
  }
  

  /************************************************************************
   * getNumPoints(): Get the number of points in the polyline.
   */

  inline int getNumPoints() const
  {
    return _points.size();
  }
  
  inline bool getSpare(const int i, fl32 &s)
  {
    if (i < 0 || i >= _numSpare)
    {
      // give a warning here
      return false;
    }

    s = _spare[i];
    return true;
  }


  /************************************************************************
   * getPoints(): Get the points in the polyline.
   */

  inline const vector< BdryPoint > &getPoints() const
  {
    return _points;
  }
  

  /************************************************************************
   * getPointsEditable(): Get the points in the polyline, in a way that they
   *                      can be editted.
   */

  inline vector< BdryPoint > &getPointsEditable()
  {
    return _points;
  }
  

  /************************************************************************
   * addPoint(): Add the given point to the end of the polyline.
   */

  inline void addPoint(const BdryPoint &point)
  {
    _points.push_back(point);
  }
  
  /************************************************************************
   * addPoint(): Add the given point to the end of the polyline.
   */

  inline void addSpare(si32 s)
  {
    if (_numSpare >= BDRY_POLYLINE_SPARE_LEN)
    {
      // give a warning here
      return;
    }
    _spare[_numSpare++] = s;
  }

  //////////////////
  // SPDB methods //
  //////////////////

  /************************************************************************
   * assemble() - Load up the given buffer from the object. Handles byte
   *              swapping.
   *
   * Returns true on success, false on failure.
   */

  bool assemble(MemBuf &mem_buf) const;
  

  /************************************************************************
   * disassemble() - Disassembles a buffer, sets the object values. Handles
   *                 byte swapping.
   *
   * Returns true on success, false on failure.
   */

  bool disassemble(char *&buf, int &len);
  

  ///////////////////////////
  // Extrapolation methods //
  ///////////////////////////

  /************************************************************************
   * extrapolate(): Extrapolate the polyline using simple extrapolation of
   *                each point in the boundary using the given speed (in m/s)
   *                and direction.
   *
   * Note: This method updates the polyline in place with the resulting
   *       extrapolation locations.
   */

  void extrapolate(const int extrap_secs,
		   const double speed, const double direction);
  

  /************************************************************************
   * extrapPointMotion():  Extrapolate the polyline using the motion vectors
   *                       associated with each point on the polyline.
   *
   * Note: This method updates the polyline in place with the resulting
   *       extrapolation locations.
   */

  void extrapPointMotion(const int extrap_secs);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /************************************************************************
   * print(): Print the boundary polyline to the given stream.
   */

  void print(FILE *stream, const bool print_points) const;

  void print(ostream &stream, const bool print_points) const;
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  int _numSecsExtrap;
  string _label;
  vector< BdryPoint > _points;
  int _numSpare;
  si32 _spare[BDRY_POLYLINE_SPARE_LEN];


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /************************************************************************
   * _polylineFromBE(): Convert a boundary polyline structure from big-endian
   *                    format to native format.
   */

  static void _polylineFromBE(BDRY_spdb_polyline_t &polyline);
  

  /************************************************************************
   * _polylineToBE(): Convert a boundary polyline structure from native
   *                  format to big-endian format.
   */

  static void _polylineToBE(BDRY_spdb_polyline_t &polyline);
  

  /************************************************************************
   * _uv2SpeedDir(): Convert U/V values to speed and direction clockwise
   *                 from true north (i.e. radar coordinates).
   *
   * Note:  u and v can be in any speed units, direction is returned in
   *        degrees and speed is returned in the same units as u and v.
   */

  static void _uv2SpeedDir(const double u, const double v,
			   double &speed, double &direction);
  

};


#endif
