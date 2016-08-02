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
// VectorsAdvector.hh
//
// VectorsAdvector class
//
// Advects a grid using U and V motion grids
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#ifndef VectorsAdvector_H
#define VectorsAdvector_H

#include <string>

#include <advect/Advector.hh>
#include <euclid/GridTemplate.hh>
#include <euclid/Pjg.hh>
#include <toolsa/mem.h>
#include <dataport/port_types.h>


class VectorsAdvector : public Advector
{
  
public:

  // constructor

  VectorsAdvector(const double vector_spacing,
		  const double smoothing_radius,
		  const bool debug_flag = false);

  // destructor
  
  virtual ~VectorsAdvector();

  // load the vector array

  bool loadVectors(const Pjg &projection,
		   const fl32 *u_data, const fl32 u_missing,
		   const fl32 *v_data, const fl32 v_missing);

  // Load the sounding to be used for the forecast.  Sounding components
  // are used to fill in the missing data spaces in the motion grid when
  // the forecast vectors are recomputed in precompute().  The sounding
  // components default to 0.0 when this object is created.
  //
  // Returns true on success, false on failure

  bool loadSounding(const double u_comp, const double v_comp);
  
  // Precompute the forecast vectors.
  //
  // Returns true on success, false on failure

  bool precompute(const Pjg &projection,
		  const int lead_time_secs);
  
  // Calculate the grid index of the original grid location from this
  // forcast grid location.
  //
  // Returns the calculated grid index if successful, returns -1 if
  // the original location is outside of the grid or if there is no
  // motion in that location.

  int calcFcstIndex(const int x_index,
		    const int y_index);
  
  // Retrieve the motion grids

  const fl32 *getUData(void) const
  {
    return _motionUData;
  }
  
  const fl32 *getVData(void) const
  {
    return _motionVData;
  }
  
  const int getNumVectors(void) const
  {
    return _nVectors;
  }
  
protected:
  
private:

  typedef struct
  {
    double lat, lon, u, v;
  } vectors_t;

  bool _debugFlag;
  
  double _vectorSpacing;
  double _smoothingRadius;
  
  int _nVectors;          // number of vectors
  vectors_t *_vectors;    // vector array
  MEMbuf *_mbuf;          // memory buffer for vector array
  bool _vectorsPrecomputed;
  
  int _leadTimeSecs;
  Pjg _motionProjection;

  mutable fl32 *_motionWtData;
  mutable fl32 *_motionUData;
  mutable fl32 *_motionVData;

  double _soundingUComp;
  double _soundingVComp;
  
  bool _loadMotionGrid(GridTemplate &grid_template,
		       const int lead_time_secs);
  
  void _loadGridForVector(const double lat, const double lon,
			  const double u, const double v,
			  GridTemplate &grid_template);
  
  static void _calcSpeedDirection(const double u, const double v,
				  double &speed, double &direction);
  
};

#endif

