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
// XyLookup tables for remapping
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
/////////////////////////////////////////////////////////////

#ifndef XY_LOOKUP_HH
#define XY_LOOKUP_HH

#include <string>
#include <vector>
#include <ctime>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include "Params.hh"
using namespace std;

class XyLookup {
  
public:
  
  typedef struct {
    unsigned long int sourceIndex;
    unsigned long int destIndex;
  } xy_lut_t;
  
  XyLookup(const Params &params,
	   const string fieldName,
	   Params::merge_method_t method,
	   const MdvxProj &outputProj);

  ~XyLookup();
  
  // check that the lookup table is correctly set up
  // if not, recompute
  
  void check(const MdvxProj &inputProj,
	     const XyLookup *master);
  
  // merge data using lookup table
  // overloaded based on encoding type
  
  void merge(int planeNum,
             const fl32 *inPlane, fl32 inMissing, fl32 inBad,
	     const time_t data_time,
	     fl32 *mergedPlane, ui08 *count, time_t *latest_time,
             const int *closestFlag);
  void merge(int planeNum,
             const ui16 *inPlane, fl32 inMissing, fl32 inBad,
	     double scale, double bias, const time_t data_time,
	     ui16 *mergedPlane, ui08 *count, time_t *latest_time,
             const int *closestFlag);
  void merge(int planeNum,
             const ui08 *inPlane, fl32 inMissing, fl32 inBad,
	     double scale, double bias, const time_t data_time,
	     ui08 *mergedPlane, ui08 *count, time_t *latest_time,
             const int *closestFlag);
  
  // set closest flag for a planea
  
  void setClosestFlag(int planeNum,
                      const fl32 *inRange,
                      fl32 rangeMissing, 
                      fl32 *closestRange,
                      int *closestFlag);

  // get field name

  const string &getFieldName() const { return _fieldName; }
  
protected:
  
private:

  const Params &_params;
  string _fieldName;
  Params::merge_method_t _method; // method for merging data
  const MdvxProj &_outputProj;    // proj for output data

  MdvxProj _inputProj;            // proj for input data

  // _lut points to the active lut
  // If this object has the same coords as the master, this will point
  // to _local in the master. If the object coords differ from the 
  // master, _lut will point to _local in this object
  
  const vector<xy_lut_t> *_lut;
  
  vector<xy_lut_t> _local;      // local lut

  // functions
  
  void _computeLookup();
  void _computeOutputBBox(int &minIxOut, int &minIyOut,
			  int &maxIxOut, int &maxIyOut);

};

#endif
