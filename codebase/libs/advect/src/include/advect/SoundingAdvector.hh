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
// SoundingAdvector.hh
//
// SoundingAdvector class
//
// Advects a grid using a sounding.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#ifndef SoundingAdvector_H
#define SoundingAdvector_H

#include <advect/Advector.hh>
#include <euclid/Pjg.hh>


class SoundingAdvector : public Advector
{
  
public:

  // constructor

  SoundingAdvector(const bool debug_flag = false);

  // destructor
  
  virtual ~SoundingAdvector();

  // load the sounding

  bool loadSounding(const double u_comp, const double v_comp);

  // Precompute the sounding advection
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
  
protected:
  
private:

  bool _debugFlag;
  
  double _uComp;
  double _vComp;
  
  Pjg _motionProjection;
  
  int _xOffset;
  int _yOffset;
  
};

#endif

