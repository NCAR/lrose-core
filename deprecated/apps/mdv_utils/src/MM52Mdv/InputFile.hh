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
// InputFile.hh
//
// Read in an MM5 file
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
/////////////////////////////////////////////////////////////

#ifndef InputFile_H
#define InputFile_H

#include "Params.hh"
#include "FlightLevel.hh"
#include <cstdio>
#include <dataport/bigend.h>
using namespace std;

class InputFile {
  
public:
  
  ///////////////
  // Constructor
  
  InputFile (char *prog_name, Params *params, char *path);

  /////////////
  // destructor

  ~InputFile();
  
  /////////////////
  // read headers()

  int readHeaders();

  ////////////////////
  // read a data set()

  int readDataset(time_t forecast_time);

  //////////////////////////
  // find the header records

  void findHdrRecords();

  // constructor status

  int OK;
  
  // grid sizes

  int nLat;
  int nLon;
  int nSigma;

  // fields - these arrays are all aligned on the cross grid.

  fl32 ***uu;
  fl32 ***vv;
  fl32 ***ww;
  fl32 ***wspd;
  fl32 ***tk;
  fl32 ***tc;
  fl32 ***qq;
  fl32 ***zz;
  fl32 ***pres;
  fl32 ***pp;
  fl32 ***rh;
  fl32 ***cloud;
  fl32 ***precip;
  fl32 ***turb;
  fl32 ***icing;

  fl32 **pstar;
  fl32 **lat;
  fl32 **lon;
  fl32 **terrain;
  fl32 **freeze;

  double *fieldInterp;

  // forecast times
  
  int nForecasts;
  time_t *forecastTimes;
  int forecastDelta;

  // interp3dField()
  //
  // Load up the sigma field array interpolated for a given point.
  //
  // returns ptr to array on success, NULL on failure.
  
  double *interp3dField(int ilat, int ilon,
			char *name, fl32 ***field,
			double wt_sw, double wt_nw,
			double wt_ne, double wt_se,
			int *sigma_needed = NULL);
  
  // interp2dField()
  //
  // Load up interp_val_p with value interpolated for a given point.
  //
  // returns val on success, MISSING_DOUBLE on failure.
  
  double InputFile::interp2dField(int ilat, int ilon,
				  char *name, fl32 **field,
				  double wt_sw, double wt_nw,
				  double wt_ne, double wt_se);
  
  // get3dScaleBias()
  //
  // Compute the scale and bias for a 3d field
  //
  // Returns 0 on success, -1 on failure
  //
  
  int get3dScaleBias(char *name, fl32 ***field,
		     double *scale_p, double *bias_p);

  // get2dScaleBias()
  //
  // Compute the scale and bias for a 2d field
  //
  // Returns 0 on success, -1 on failure
  //
  
  int get2dScaleBias(char *name, fl32 **field,
		     double *scale_p, double *bias_p);

protected:
  
private:

  char *_progName;
  Params *_params;
  char *_path;

  unsigned int _nyDot;
  unsigned int _nxDot;
  int _nPtsDotPlane;

  int _n3d;
  int _n2d;

  int _uField;
  int _vField;
  int _tField;
  int _qField;
  int _clwField;
  int _rnwField;
  int _wField;
  int _ppField;
  
  int _pstarField;
  int _terrainField;
  int _latField;
  int _lonField;
  
  int _fieldDataLen;
  int _datasetLen;

  fl32 _pTop;
  fl32 _pos;
  fl32 _tso;
  fl32 _tlp;

  si32 **_mif;
  fl32 **_mrf;
  fl32 *_halfSigma;

  fl32 ***_field3d;
  fl32 **_field2d;

  FILE *_in;

  FlightLevel *_flevel;
  
  // functions

  void _print();
  void _readFortRecLen();
  int _readHdrInfo();
  int _readTimes();
  long _offset3d(int data_set_num, int field_num_3d);
  long _offset2d(int data_set_num, int field_num_2d);

  int _read2dField(int data_set_num, int field_num_2d,
		   char *field_name, fl32 ***field_p);

  int _read3dField(int data_set_num, int field_num_3d,
		   char *field_name, fl32 ****field_p);

  int _readwField(int data_set_num);

  void _changeMixrUnits();

  void _loadTempCField();

  void _loadPressureField();

  void _loadRhField();

  void _loadWspdField();

  void _loadIcingField();

  void _loadTurbField();
  
  void _loadFreezeField();

};

#endif
