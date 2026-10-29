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
// MM5DataV3.hh
//
// Read in a V3 MM5 file
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
/////////////////////////////////////////////////////////////

#ifndef MM5DataV3_H
#define MM5DataV3_H


#include "MM5Data.hh"
using namespace std;

#define MIF_N1 20
#define MIF_N2 50
#define MIF_N 1000
#define MRF_N1 20
#define MRF_N2 20
#define MRF_N 400

#define MM5_HEADER_LEN 117600

class MM5DataV3 : public MM5Data {
  
public:

  typedef char label_t[80];

  typedef struct {
    si32 ndim;
    si32 start_index[4];
    si32 end_index[4];
    fl32 time;
    char staggering[4];
    char ordering[4];
    char current_date[24];
    char name[9];
    char units[25];
    char descr[46];
  } sub_header_t;

  ///////////////
  // Constructor
  //
  // If flight_level is non-NULL, the freezing level is computed
  // as flight level.
  // If flight_level is NULL, the freezing level is computed as mb.
  // If you want registration with the procmap, set the heartbeat_func
  //    to PMU_auto_register() from toolsa.
  
  MM5DataV3 (const string &prog_name,
	     const string &path,
	     bool debug = false,
	     const heartbeat_t heartbeat_func = NULL,
	     bool dbzConstantIntercepts = true);

  /////////////
  // destructor

  ~MM5DataV3();

  ////////////////
  // print headers

  virtual void printHeaders(FILE *out) const;

  /////////////////
  // read file

  virtual int read();

  //////////////////////////
  // find the header records

  virtual void findHdrRecords();

  /////////////////////////////////////////
  // get header and data set len and offset
  
  long getHeadersLen() { return _headersLen; }
  long getHeadersOffset() { return _headersOffset; }
  long getDataSetLen() { return _dataSetLen; }
  long getDataSetOffset() { return _dataSetOffset; }

  // get the FDDA start and end times

  virtual double getFddaStartTime() const;
  virtual double getFddaEndTime() const;

  // get MIF or MRF values, given the label

  virtual int getMifVal(const char *label) const;
  virtual double getMrfVal(const char *label) const;

protected:
  
private:

  label_t **_mifc;
  label_t **_mrfc;
  int _sigmaLevelField;

  int file_type; // indicating MM5INPUT or MM5OUTPUT

  long _headersOffset;
  int _headersLen;
  long _dataSetOffset;
  int _dataSetLen;

  // functions

  int _readHeaders();

  int _readDataset();

  void _setTimes();

  int _read2dField(int data_set_num, int field_num_2d,
		   char *field_name, fl32 ***field_p);

  void _load3dField(fl32 ****field_p, bool is_dot);

  void _load3dField2Dot(fl32 ****field_p);

  void _load2dField(fl32 ***field_p, bool is_dot);

  void _load2dField2Dot(fl32 ***field_p);

  void _loadWField();

  int _loadSigmaLevels(int nz, fl32 *fieldData);

  int _check3dSize(int nz, int ny, int nx, const char *field_name);

  int _check2dSize(int ny, int nx, const char *field_name);

};

#endif













