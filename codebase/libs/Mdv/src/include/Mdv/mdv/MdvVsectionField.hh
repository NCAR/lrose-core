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

//////////////////////////////////////////////////////////
// mdv/MdvVsectionField.hh
//
// Class for representing MDV sert sections.
// Used by MdvVsection.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
///////////////////////////////////////////////////////////

#ifndef MdvVsectionField_hh
#define MdvVsectionField_hh

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_grid.h>
#include <string>
#include <vector>
using namespace std;

class MdvVsectionField {

  friend class MdvVsection;
  friend class DsMdvMsg;

public:

  // default constructor
  // If this is used, you must call init() before using the object
  
  MdvVsectionField();

  // primary constructor
  
  MdvVsectionField(const int field_num);

  // copy constructor

  MdvVsectionField(const MdvVsectionField &other);

  // destructor

  virtual ~MdvVsectionField();

  // initialization - must br called if the default constructor is used
  
  void init(const int field_num);

  // assignment
  
  void operator=(const MdvVsectionField &other);

  // Alloc memory for vsection
  
  void allocData(int n_levels, int n_pts,
		 int elem_size, int encoding_type = MDV_INT8);

  // Free up memory associated with grids
  
  void freeData();

  // print summary

  void printSummary(ostream &out);

  // print data

  void printData(ostream &out);

  // access to data members

  int                   getFieldNum() const { return (_fieldNum); }
  MDV_field_header_t &  getFieldHeader()    { return (_fieldHeader); }
  MDV_vlevel_header_t & getVlevelHeader()   { return (_vlevelHeader); }

  int getDataElemSize() { return (_dataElemSize); }
  int getDataEncodingType() { return (_dataEncodingType); }
  int getnLevels() { return (_nLevels); }
  int getnPts() { return (_nPts); }

  void  *getData1D()           { return (_data1D); }
  void **getData2D()           { return (_data2D); }

protected:
  
  bool _initDone;
  int _fieldNum;

  MDV_field_header_t _fieldHeader;
  MDV_vlevel_header_t _vlevelHeader;

  // Vsection field data
  // 1D data is contiguous, and 2D array points into it.

  int _dataElemSize;
  int _dataEncodingType;
  int _nLevels;
  int _nPts;
  
  void *_data1D;
  void **_data2D;

private:

};

#endif


