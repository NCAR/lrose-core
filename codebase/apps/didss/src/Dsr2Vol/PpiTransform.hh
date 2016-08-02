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
// PpiTransform.hh
//
// Cartesian PPI transform object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////
//
// Transform performs the geometric transformation from radar coords
// into the required PPI coords for the MDV file.
//
//////////////////////////////////////////////////////////////

#ifndef PpiTransform_H
#define PpiTransform_H

#include "PlanTransform.hh"
using namespace std;

class PpiTransform : public PlanTransform

{
  
public:
  
  PpiTransform (const Params &params,
		const string &mdv_url,
                const vector<FieldInfo> &fields,
		int nxy, double dxy,
		double min_elev, double max_elev,
		bool interp,
		double min_ht, double max_ht,
		double max_range);

  virtual ~PpiTransform();
  
  virtual bool setGeom(const vector<double> elevations,
                       int n_gates,
                       double start_range,
                       double gate_spacing,
                       int n_az,
                       int n_az_per_45,
                       double delta_az,
                       double beam_width,
                       double radar_lat,
                       double radar_lon,
                       double radar_alt);

  virtual void calcLookup();

  virtual void doTransform(double nyquist);

protected:

  bool _interp;
  int _nxy;
  int _nxyHalf;
  double _dxy;
  double _minElev;
  double _maxElev;
  double _minHt;
  double _maxHt;
  int _elevOffset;
  double _maxRange;

  void _transformInterp(double nyquist);
  void _transformNearest();


};

#endif

